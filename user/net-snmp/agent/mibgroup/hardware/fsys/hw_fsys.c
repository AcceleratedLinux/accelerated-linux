#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/hardware/fsys.h>


extern void             netsnmp_fsys_arch_load( void );
extern void             netsnmp_fsys_arch_init( void );
static int  _fsys_load( void );
static void _fsys_free( void );

static int _fsysAutoUpdate = 0;   /* 0 means on-demand caching */
static void _fsys_update_stats( unsigned int, void* );

netsnmp_cache     *_fsys_cache     = NULL;
netsnmp_container *_fsys_container = NULL;
static int         _fsys_idx       = 0;
static netsnmp_fsys_info * _fsys_create_entry( void );

void init_hw_fsys( void ) {

    if ( _fsys_container )
        return;   /* Already initialised */

    DEBUGMSGTL(("fsys", "Initialise Hardware FileSystem module\n"));

    /*
     * Define a container to hold the list of filesystems
     */
    _fsys_container = netsnmp_container_find("fsysTable:table_container");
    if ( NULL == _fsys_container ) {
        snmp_log( LOG_ERR, "failed to create container for fsysTable");
        return;
    }
    netsnmp_fsys_arch_init( );

    /*
     * If we're sampling the file system information automatically,
     *   then arrange for this to be triggered regularly.
     *
     * If we're not sampling these values regularly,
     *   create a suitable cache handler instead.
     */
    if ( _fsysAutoUpdate ) {
        DEBUGMSGTL(("fsys", "Reloading Hardware FileSystems automatically (%d)\n",
                               _fsysAutoUpdate));
        snmp_alarm_register( _fsysAutoUpdate, SA_REPEAT,
                             _fsys_update_stats, NULL );
    }
    else {
        _fsys_cache = netsnmp_cache_create( 5, netsnmp_fsys_load,
                                               netsnmp_fsys_free, NULL, 0 );
        DEBUGMSGTL(("fsys", "Reloading Hardware FileSystems on-demand (%p)\n",
                               _fsys_cache));
    }
}

void shutdown_hw_fsys( void ) {
    _fsys_free();
}

/*
 *  Return the main fsys container
 */
netsnmp_container *get_fsys_container( void ) { return _fsys_container; }

/*
 *  Return the main fsys cache control structure (if defined)
 */
netsnmp_cache *get_fsys_cache( void ) { return _fsys_cache; }


/*
 * Wrapper routine for automatically updating fsys information
 */
void
_fsys_update_stats( unsigned int clientreg, void *data )
{
    _fsys_free();
    _fsys_load();
}

/*
 * Wrapper routine for re-loading filesystem statistics on demand
 */
int
netsnmp_fsys_load( netsnmp_cache *cache, void *data )
{
    return _fsys_load();
}

/*
 * Wrapper routine for releasing expired filesystem statistics
 */
void
netsnmp_fsys_free( netsnmp_cache *cache, void *data )
{
    _fsys_free();
}


/*
 * Architecture-independent processing of loading filesystem statistics
 */
static int
_fsys_load( void )
{
    netsnmp_fsys_arch_load();
}

/*
 * Architecture-independent release of filesystem statistics
 */
static void
_fsys_free( void )
{
    netsnmp_fsys_info *sp;

    for (sp = CONTAINER_FIRST( _fsys_container );
         sp;
         sp = CONTAINER_NEXT(  _fsys_container, sp )) {

         sp->flags &= ~NETSNMP_FS_FLAG_ACTIVE;
    }
}


netsnmp_fsys_info *netsnmp_fsys_get_first( void ) {
    return CONTAINER_FIRST( _fsys_container );
}
netsnmp_fsys_info *netsnmp_fsys_get_next( netsnmp_fsys_info *this_ptr ) {
    return CONTAINER_NEXT( _fsys_container, this_ptr );
}

/*
 * Retrieve a filesystem entry based on the path where it is mounted,
 *  or (optionally) insert a new one into the container
 */
netsnmp_fsys_info *
netsnmp_fsys_by_path( char *path, int create_type )
{
    netsnmp_fsys_info *sp;

    DEBUGMSGTL(("fsys:path", "Get filesystem entry (%s)\n", path));

    /*
     *  Look through the list for a matching entry
     */
        /* .. or use a secondary index container ?? */
    for (sp = CONTAINER_FIRST( _fsys_container );
         sp;
         sp = CONTAINER_NEXT(  _fsys_container, sp )) {

        if ( !strcmp( path, sp->path ))
            return sp;
    }

    /*
     * Not found...
     */
    if ( create_type == NETSNMP_FS_FIND_EXIST ) {
        DEBUGMSGTL(("fsys:path", "No such filesystem entry\n"));
        return NULL;
    }

    /*
     * ... so let's create a new one
     */
    sp = _fsys_create_entry();
    if ( sp ) {
        strcpy( sp->path, path );
    }
    return sp;
}


/*
 * Retrieve a filesystem entry based on the hardware device,
 *   (or exported path for remote mounts).
 * (Optionally) insert a new one into the container.
 */
netsnmp_fsys_info *
netsnmp_fsys_by_device( char *device, int create_type )
{
    netsnmp_fsys_info *sp;

    DEBUGMSGTL(("fsys:device", "Get filesystem entry (%s)\n", device));

    /*
     *  Look through the list for a matching entry
     */
        /* .. or use a secondary index container ?? */
    for (sp = CONTAINER_FIRST( _fsys_container );
         sp;
         sp = CONTAINER_NEXT(  _fsys_container, sp )) {

        if ( !strcmp( device, sp->device ))
            return sp;
    }

    /*
     * Not found...
     */
    if ( create_type == NETSNMP_FS_FIND_EXIST ) {
        DEBUGMSGTL(("fsys:device", "No such filesystem entry\n"));
        return NULL;
    }

    /*
     * ... so let's create a new one
     */
    sp = _fsys_create_entry();
    if ( sp ) {
        strcpy( sp->device, device );
    }
    return sp;
}


netsnmp_fsys_info *
_fsys_create_entry( void )
{
    netsnmp_fsys_info *sp;

    sp = SNMP_MALLOC_TYPEDEF( netsnmp_fsys_info );
    if ( sp ) {
        /*
         * Set up the index value.
         *  
         * All this trouble, just for a simple integer.
         * Surely there must be a better way?
         */
        sp->idx.len  = 1;
        sp->idx.oids = SNMP_MALLOC_TYPEDEF( oid );
        sp->idx.oids[0] = ++_fsys_idx;
    }

    DEBUGMSGTL(("fsys:new", "Create filesystem entry (index = %d\n", _fsys_idx));
    CONTAINER_INSERT( _fsys_container, sp );
    return sp;
}


/*
 *  Convert fsys size information to 1K units
 *    (attempting to avoid 32-bit overflow!)
 */
unsigned int
_fsys_to_K( int size, int units )
{
    int factor = 1;

    if ( units == 0 ) {
        return 0;    /* XXX */
    } else if ( units == 1024 ) {
        return size;
    } else if ( units == 512 ) {      /* To avoid unnecessary division */
        return size/2;
    } else if ( units < 1024 ) {
        factor = 1024 / units;   /* Assuming power of two */
        return (size * factor);
    } else {
        factor = units / 1024;   /* Assuming multiple of 1K */
        return (size / factor);
    }
}

unsigned int
netsnmp_fsys_size( netsnmp_fsys_info *f) {
    if ( !f ) {
        return 0;
    }
    return _fsys_to_K( f->size, f->units );
}

unsigned int
netsnmp_fsys_used( netsnmp_fsys_info *f) {
    if ( !f ) {
        return 0;
    }
    return _fsys_to_K( f->used, f->units );
}

unsigned int
netsnmp_fsys_avail( netsnmp_fsys_info *f) {
    if ( !f ) {
        return 0;
    }
    return _fsys_to_K( f->avail, f->units );
}
