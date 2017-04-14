#!/bin/bash

delete_archives_before=$(date --date="3 days ago" +"%Y%m%d")
echo "$delete_archives_before"

# loop through archives and parse for date
saved_archives=($(ls /afs/ir/group/webservices/cgi-bin/delete-modules))
for archive in "${saved_archives[@]}"; do
  # parse date
  if [[ "$archive" == *"archives-"* ]]; then
    archive_saved_on=$(echo "$archive" | sed -e 's/archives-//')
    if [ $delete_archives_before -ge $archive_saved_on ]; then
      echo "deleting: $archive, saved on: $archive_saved_on, deleting archives before: $delete_archives_before" >> archives_deleted.log
    fi
  fi
done
