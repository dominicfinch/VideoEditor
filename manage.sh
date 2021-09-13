script_dir=$(dirname "$0")

function Usage() {
  echo "TODO"
}

if [ "$1" = "help" ]; then
  Usage
else
    if [ -z "$1" ]; then
        :
    else
        echo "Unrecognised argument: $1"
    fi;
fi;