export QTDIR="/Library/Qt/5.4/clang_64"
QTLANGDIR=$QTDIR/translations
translationsdir="../translations"

function convertLang {
	if [ -f $QTLANGDIR/qtbase_$1.qm ];
	then
		$QTDIR/bin/lconvert -o $translationsdir/qt_$1.qm $QTLANGDIR/qtbase_$1.qm $QTLANGDIR/qt_$1.qm
	else
		echo "Warning: Language $1 not translated by qt"
		$QTDIR/bin/lconvert -o $translationsdir/qt_$1.qm $QTLANGDIR/qt_$1.qm
	fi
}

convertLang "de"
convertLang "pl"
convertLang "es"
convertLang "ru"
convertLang "pt"