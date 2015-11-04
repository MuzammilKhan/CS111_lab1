#! /bin/sh

# UCLA CS 111 Lab 1 - Test for executing commands

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >testtemp.sh <<'EOF'

cat < a.txt

cat < a.txt

cat > b.txt

cat > a.txt

ls < a.txt

cat < b.txt

ls

cat < a.txt > b.txt

ls

EOF



../timetrash -t testtemp.sh  || exit

) || exit

rm -fr "$tmp"
