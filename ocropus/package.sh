#! /bin/sh

V='ocropus-0.2'

T=`mktemp`
D=$T-ocropus
mkdir $D
cd $D
svn checkout http://ocropus.googlecode.com/svn/trunk ocropus
rm -f ocropus/check-* ocropus/testing/check-*
rm -f ocropus/fix-include-dependencies ocropus/DIRS
rm -f ocropus/package.sh
cat >ocropus/Jamrules <<"EOF"
if $(TOP) = "." {
    Exit "Please run\
    ./configure" ;
} else {
    Exit "Please run\
    cd $(TOP)\
    ./configure" ;
}
EOF
mv ocropus $V
find -name ".svn" -exec rm -rf {} ";"
tar czf $V.tar.gz $V
cd -
mv $D/$V.tar.gz .
rm -rf $D
# echo $D
rm -f $T
