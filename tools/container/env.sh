if [ "$SHELL" != /bin/bash ]
then
    echo "Sorry, this script requires bash" >&2
    exit 1

elif [ "$0" = "$BASH_SOURCE" ]
then
    echo "Please source this script instead of running it" >&2
    exit 1
fi

FN_NAME=dal
COMPOSE_FILE=docker-compose.yml
COMPOSE_SERVICE=dal

COMPOSE_DIR="$(dirname "${BASH_SOURCE[0]}")"
COMPOSE_PATH=$(readlink -f "$COMPOSE_DIR/$COMPOSE_FILE")

function docker_run()
{
    compose_path=$1
    shift
    compose_service=$1
    shift

    docker-compose -f "$compose_path" run --rm $compose_service "$@"
}

if [ -f "$COMPOSE_PATH" ]
then
    # This construction ensures the values of COMPOSE_PATH and COMPOSE_SERVICE
    # are fixed when the function is defined rather than when it is used. This
    # way we can potentially define multiple similar functions associated with
    # other projects in the same shell.
    eval $(cat <<-EOT
		function $FN_NAME()
		{
		    docker_run "$COMPOSE_PATH" $COMPOSE_SERVICE "\$@";
		}
		EOT
    )
fi

unset FN_NAME
unset COMPOSE_FILE
unset COMPOSE_SERVICE
unset COMPOSE_DIR
unset COMPOSE_PATH
