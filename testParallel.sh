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

cat >words2.txt << 'EOF'
sky
air
land
sea
EOF


cat >testtemp.sh <<'EOF'

cat words.txt

cat words2.txt

cat words.txt > words2.txt

ls words.txt

cat words2.txt

echo hello3

echo "quotation test" > quote.txt

cat quote.txt

EOF



../timetrash -t testtemp.sh  || exit

) || exit

rm -fr "$tmp"
