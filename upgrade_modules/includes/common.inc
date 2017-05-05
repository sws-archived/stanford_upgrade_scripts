#############
# FUNCTIONS #
#############

# bash function like PHP's in_array(). See http://eqcode.com/wiki/In_array
function in_array() {
  NEEDLE="$1"; shift; ARRAY=("$@")
  # adjusted to remove quotes from status_selection and difference_selection array values
  for VALUE in ${ARRAY[@]}; do [ "$VALUE" == "$NEEDLE" ] && echo 1 && return; done
  echo 0
}

function check_exit_status {
  exitstatus=$?
  [[ "$exitstatus" == 1 ]] && exit
}

## Compares the md5 hash of two .info files for version sameness
## See /afs/ir/group/webservices/tools/jumpstart-4.0/includes/common.inc
function get_excludes {
  ROOTPATH=$(pwd)
  DESTINATION=$1
  SOURCE=$2
  EXCLUDE=''

  cd $DESTINATION
  files=(*/)
  cd $ROOTPATH
  for dir in "${files[@]}";
    do
    destpath=$DESTINATION$dir${dir%?}'.info';
    sourcepath=$SOURCE$dir${dir%?}'.info';

    if [ ! -f $sourcepath ]; then
      continue
    fi

    sum1=$(md5sum $destpath | awk '{ print $1 }')
    sum2=$(md5sum $sourcepath | awk '{ print $1 }')

    if [[ "$sum1" == "$sum2" ]]; then
      EXCLUDE+="--exclude=/"${dir%?}" "
    fi

  done

  echo $EXCLUDE
}

## Compares two directories for sameness, using recursive diff
## See /afs/ir/group/webservices/tools/jumpstart-4.0/includes/common.inc
function get_excludes_directories {
  ROOTPATH=$(pwd)
  DESTINATION=$1
  SOURCE=$2
  EXCLUDE=''

  cd $DESTINATION
  files=(*/)
  cd $ROOTPATH
  for dir in "${files[@]}";
    do
    destpath=$DESTINATION$dir;
    sourcepath=$SOURCE$dir;

    if [ ! -d $sourcepath ]; then
      continue
    fi

    diff=$(diff $destpath $sourcepath -bqrd)

    if [ -z "$diff" ]; then
      EXCLUDE+="--exclude=/"${dir%?}" "
    fi

  done

  echo $EXCLUDE
}
