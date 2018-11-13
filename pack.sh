#!/bin/bash

export file_list='*.cpp util.h pack.sh Makefile relatorio.pdf docs/*'
export zip_name="$1"

now (){
	echo "$(date +'%y%m%d')"
}
ed_zip(){
	zip_name="$1"
	zip -u $zip_name $file_list
}

echo "Zipping files for mailing..."
echo "Today is $(date)"
rm *$zip_name.zip
ed_zip "$(now)-$zip_name.zip"