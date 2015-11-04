#! /bin/sh

# UCLA CS 111 Lab 1 - Test for executing commands

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >testtemp.sh <<'EOF'

cat > b.txt || ls > c.txt

cat a.txt < b.txt

(cat) > b.txt

cat > a.txt

cat a.txt c.txt

ls < a.txt

cat < b.txt

ls > c.txt | cat && cat b.txt

cat < a.txt > b.txt

ls

EOF



../timetrash -t testtemp.sh  || exit

) || exit

rm -fr "$tmp"
