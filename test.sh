#! /bin/sh

# UCLA CS 111 Lab 1 - Test for executing commands

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
ls | sort

ls || sort

ls && sort
EOF



../timetrash -p test.sh  || exit

) || exit

rm -fr "$tmp"
