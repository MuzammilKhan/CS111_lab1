#! /bin/sh

# UCLA CS 111 Lab 1 - Test for executing commands

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >words.txt <<'EOF'
monkey
cat
dog
bird
chicken
bull
ant
EOF

cat >testtemp.sh <<'EOF'

cat < words.txt

echo hello

echo hello > words.txt

cat words.txt

EOF



../timetrash -t testtemp.sh  || exit

) || exit

rm -fr "$tmp"
