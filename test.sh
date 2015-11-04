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
ls | sort

ls || sort

cat < words.txt;
(sort words.txt)

echo hello

ls && ls

(echo done);(echo now)

(cat) < words.txt
EOF



../timetrash testtemp.sh  || exit

) || exit

rm -fr "$tmp"
