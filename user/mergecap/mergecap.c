/*
 * mergecap utility
 * -----------------
 *
 * Author: Jeff Shaw
 * Accelerated Pty Ltd 2018
 *
 * Merges two pcap files (from tcpdump) into one according to the timestamp.
 * It can take files from two different interfaces and merge them into a single
 * pcap file. This allows the following scenario:
 *
 * Test routing - client on lan1 pings a machine on wan, eg
 *
 * client = 192.168.1.2 on lan1 (192.168.1.0/24)
 * target = 10.1.1.1 on wan
 *
 * NOTE: both addresses must exist, we are capruring real traffic
 *
 * tcpdump -i lan1 icmp and dst host 10.1.1.1 -c 5 -w lan1.dump &
 * tcpdump -i wan icmp and srt host 10.1.1.1 -c 5 -w wan.dump &
 *
 * To test, we want to ping a non-existant machine 10.1.1.2, so we
 * modify the dumps with tcprewrite.
 *
 * tcprewrite -D 0.0.0.0/0:10.1.1.2 --infile lan1.dump --outfile lan1.pcap
 * tcprewrite -S 0.0.0.0/0:10.1.1.2 --infile wan.dump --outfile wan.pcap
 *
 * we could also change the server (NAT) address from the ping reply if we wanted.
 *
 * Now, merge the files:
 * mergecap lan1.pcap wan.pcap ping_test.temp
 *
 * Update the checksums:
 * tcprewrite --fixcsum --infile ping_test.temp --outfile ping_test.pcap
 *
 * Generate a cache file so that tcpreplay can use multiple interfaces:
 * tcpprep --auto=first --cachefile=in.cache --pcap ping_test.pcap
 *
 * Now run tcpreplay!
 * tcpreplay --intf1=lan1 --intf2=wan --cachefile=in.cache ping_test.pcap
 *
 */
#include<stdio.h>
#include<stdlib.h>

#define OK 0
#define ERROR_ARGS 1
#define ERROR_FILE 2
#define ERROR_HEADER 3
#define ERROR_OUTPUT 4
#define ERROR_MEMORY 5

#define false 0
#define true 1

/*
 * ARM needs long long for 64 bit
 */
#define FINISHED 0xffffffffffffffffULL

/*
 * Header of a pcap file
 */
typedef struct _dump_header {
	unsigned int magic;
	unsigned int version; // 0xmmMM
	unsigned int tz_offset;
	unsigned int tz_accuracy;
	unsigned int snapshot_length;
	unsigned int link_layer_header;
} dump_header;

/*
 * Header of a packet inside the pcap file
 */
typedef struct _packet_header {
	unsigned int seconds;
	unsigned int microseconds;
	unsigned int cap_length;
	unsigned int orig_length;
} packet_header;

/*
 * Encapsulated tcpdump file
 */
typedef struct _tcpdump {
	FILE *file;
	unsigned long long timestamp;
	packet_header header;
	struct _tcpdump *next;
} tcpdump;

/*
 * Close all open files, free the list and exit
 */
void close_and_exit(tcpdump *head, int exit_code) {
	tcpdump *old;
	while(head) {
		if(head->file)
			fclose(head->file);
		old = head;
		head = head->next;
		free(old);
	}
	exit(exit_code);
}

/*
 * The pcap header starts with 0xa1b2c3d4
 */
char verify_header(FILE *file, dump_header *header)
{
	if (!file)
		return false;

	fread(header, sizeof(dump_header), 1, file);
	if (header->magic != 0xa1b2c3d4)
		return false;
	return true;
}

/*
 * Get the timestamp as a 64 bit number made up of seconds since epoch and microseconds this second.
 */
unsigned long long get_timestamp(FILE *file, packet_header *header)
{
	if (fread(header, sizeof(packet_header), 1, file) != 1) {
		return FINISHED;
	}
	return ((unsigned long long)header->seconds << 32) + (unsigned long long)header->microseconds;
}

/*
 * Create an entry in the linked list for the specified tcpdump filename
 */
int create_entry(tcpdump **current, char *filename) {
	tcpdump *new_entry;
	if((new_entry = (tcpdump *)malloc(sizeof(tcpdump))) == NULL)
		return ERROR_MEMORY;
	if(*current)
		(*current)->next = new_entry;
	*current = new_entry;

	(*current)->next = NULL;

	if (((*current)->file = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Couldn't open file: %s\n", filename);
		return ERROR_FILE;
	}

	return OK;
}

/*
 * Are there any streams still with data?
 */
char any_left(tcpdump *head) {
	while(head) {
		if(head->timestamp != FINISHED)
			return true;
		head = head->next;
	}
	return false;
}

/*
 * Which object has the next timestamp
 */
tcpdump *get_lowest_timestamp(tcpdump *head) {
	tcpdump *winner = head;
	while(head->next) {
		head = head->next;
		if(head->timestamp < winner->timestamp) {
			winner = head;
		}
	}
	return winner;
}

int main(int argc, char *argv[]) {
	tcpdump *head = NULL, *current = NULL;
	FILE *output;
	dump_header dump;
	unsigned char *buffer;
	int loop, result;

	/*
	 * We need at minimum 3 args - file1, file2 and output
	 */
	if (argc < 4) {
		fprintf(stderr, "Usage: %s <file1> <file2> [file3 [file4 [...]]] <output>\n", argv[0]);
		exit(ERROR_ARGS);
	}

	/*
	 * Loop through the input files, verify them and create the linked list
	 */
	for(loop = 1; loop < (argc-1); loop++) {
		if(!head) {
			result = create_entry(&head, argv[loop]);
			current = head;
		}
		else
			result = create_entry(&current, argv[loop]);

		if(result != OK)
			close_and_exit(head, result);

		if (!verify_header(current->file, &dump)) {
			fprintf(stderr, "%s doesn't appear to be a tcpdump file\n", argv[loop]);
			close_and_exit(head, ERROR_HEADER);

		}
		current->timestamp = get_timestamp(current->file, &(current->header));
	}

	/*
	 * Allocate the buffer for reading packets
	 */
	if ((buffer = malloc(dump.snapshot_length)) == NULL) {
		fprintf(stderr, "Can't allocate %d bytes for the buffer\n", dump.snapshot_length);
		close_and_exit(head, ERROR_MEMORY);
	}

	/*
	 * Open the output file
	 */
	if ((output = fopen(argv[argc-1], "w")) == NULL) {
		fprintf(stderr, "Couldn't open output file: %s\n", argv[argc-1]);
		free(buffer);
		close_and_exit(head, ERROR_OUTPUT);
	}

	/*
	 * It doesn't matter which header we use, they should be the same.
	 */
	fwrite(&dump, sizeof(dump_header), 1, output);

	/*
	 * Loop until all files are finished. Take the next timestamp from
	 * the list, and update the corresponding timestamp
	 */
	while (any_left(head) == true) {
		current = get_lowest_timestamp(head);
		fwrite(&(current->header), sizeof(packet_header), 1, output);
		fread(buffer, current->header.cap_length, 1, current->file);
		fwrite(buffer, current->header.cap_length, 1, output);
		current->timestamp = get_timestamp(current->file, &(current->header));
	}
	fclose(output);
	free(buffer);
	close_and_exit(head, OK);
}
