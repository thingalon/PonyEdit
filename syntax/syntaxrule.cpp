#include "syntax/syntaxrule.h"
#include "main/tools.h"

QMap<QString, SyntaxRule::Type> SyntaxRule::sTypeMap;
bool SyntaxRule::sTypeMapInitialized = false;

SyntaxRule::SyntaxRule(SyntaxRule* parent, const QString& name, const QXmlAttributes& attributes)
{
	if (!sTypeMapInitialized)
	{
		sTypeMap.insert("detectchar", DetectChar);
		sTypeMap.insert("detect2chars", Detect2Chars);
		sTypeMap.insert("anychar", AnyChar);
		sTypeMap.insert("stringdetect", StringDetect);
		sTypeMap.insert("worddetect", WordDetect);
		sTypeMap.insert("regexpr", RegExpr);
		sTypeMap.insert("keyword", Keyword);
		sTypeMap.insert("int", Int);
		sTypeMap.insert("float", Float);
		sTypeMap.insert("hlcoct", HlCOct);
		sTypeMap.insert("hlchex", HlCHex);
		sTypeMap.insert("hlcstringchar", HlCStringChar);
		sTypeMap.insert("hlcchar", HlCChar);
		sTypeMap.insert("rangedetect", RangeDetect);
		sTypeMap.insert("linecontinue", LineContinue);
		sTypeMap.insert("detectspaces", DetectSpaces);
		sTypeMap.insert("detectidentifier", DetectIdentifier);
		sTypeMap.insert("includerules", IncludeRules);
		sTypeMapInitialized = true;
	}

	mName = name;
	mParent = parent;
	mValid = false;

	QString lcName = name.toLower();
	if (sTypeMap.contains(lcName))
	{
		mType  = sTypeMap.value(lcName);

		if (mType == IncludeRules && mParent != NULL)
		{
			qDebug() << "Warning: Include inside parent rule; disregarding!";
			return;
		}

		mAttribute = Tools::getStringXmlAttribute(attributes, "attribute");
		mContext = Tools::getStringXmlAttribute(attributes, "context");
		mBeginRegion = Tools::getStringXmlAttribute(attributes, "beginregion");
		mEndRegion = Tools::getStringXmlAttribute(attributes, "endregion");
		mLookAhead = Tools::getIntXmlAttribute(attributes, "lookahead", 0);
		mFirstNonSpace = Tools::getIntXmlAttribute(attributes, "firstnonspace", 0);
		mColumn = Tools::getIntXmlAttribute(attributes, "column", 0);
		mCharacterA = Tools::getCharXmlAttribute(attributes, "char");
		mCharacterB = Tools::getCharXmlAttribute(attributes, "char1");
		mString = Tools::getStringXmlAttribute(attributes, "string");
		mCaseInsensitive = Tools::getIntXmlAttribute(attributes, "insensitive", 0);
		mDynamic = Tools::getIntXmlAttribute(attributes, "dynamic", 0);
		mMinimal = Tools::getIntXmlAttribute(attributes, "minimal", 0);
		mIncludeAttrib = Tools::getIntXmlAttribute(attributes, "includeAttrib", 0);

		mValid = true;
	}
	else
	{
		qDebug() << "Unrecognized rule type: " << name;
	}
}

SyntaxRule::~SyntaxRule()
{
}

void SyntaxRule::addChildRule(SyntaxRule* rule)
{
	mChildRules.append(rule);
}

bool SyntaxRule::link(SyntaxDefinition* def)
{
	if (mAttribute.isEmpty())
		mAttributeLink = NULL;
	else
	{
		mAttributeLink = def->getItemData(mAttribute);
		if (!mAttributeLink)
		{
			qDebug() << "Failed to link attribute: " << mAttribute;
			return false;
		}
	}

	//	Context can be "#stay" (no change), "#pop#pop..." (pop x times), or a context name
	mContextLink = NULL;
	mContextPopCount = 0;
	if (mContext.startsWith('#') || mContext.isEmpty())
	{
		if (mContext.startsWith("#pop", Qt::CaseInsensitive))
			mContextPopCount = mContext.count('#');
	}
	else
	{
		mContextLink = def->getContext(mContext);
		if (!mContextLink)
		{
			qDebug() << "Failed to link context: " << mContext;
			return false;
		}
	}

	//	Link all children too
	foreach (SyntaxRule* rule, mChildRules)
		if (!rule->link(def))
			return false;

	//	Rule specific link-ups
	switch (mType)
	{
	case RegExpr:
		mRegExp = QRegExp(mString, mCaseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive);
		break;

	case Keyword:
		mKeywordLink = def->getKeywordList(mString);
		if (!mKeywordLink)
		{
			qDebug() << "Failed to link keyword list: " << mString;
			return false;
		}
		break;

	default:break;
	}

	if (mCaseInsensitive)
	{
		mString = mString.toLower();
		mCharacterA = mCharacterA.toLower();
		mCharacterB = mCharacterB.toLower();
	}

	return true;
}

int SyntaxRule::match(const QString &string, int position)
{
	int match = 0;

	switch (mType)
	{
	case DetectChar:
		if (mCaseInsensitive)
			match = (string.at(position).toLower() == mCharacterA ? 1 : 0);
		else
			match = (string.at(position) == mCharacterA ? 1 : 0);
		break;

	case Detect2Chars:
		if (position < string.length() - 1)
		{
			if (mCaseInsensitive)
				match = (string.at(position).toLower() == mCharacterA && string.at(position + 1).toLower() == mCharacterB ? 2 : 0);
			else
				match = (string.at(position) == mCharacterA && string.at(position + 1) == mCharacterB ? 2 : 0);
		}
		break;

	case AnyChar:
		if (mString.contains(string.at(position)))
			match = 1;
		break;

	case StringDetect:
		if (Tools::compareSubstring(string, mString, position, mCaseInsensitive?Qt::CaseInsensitive:Qt::CaseSensitive))
			match = mString.length();
		break;

	case WordDetect:
		if (position == 0 || string.at(position - 1).isSpace())	// space before
			if (position == string.length() - mString.length() || string.at(position + mString.length()).isSpace())
				if (Tools::compareSubstring(string, mString, position, mCaseInsensitive?Qt::CaseInsensitive:Qt::CaseSensitive))
					match = mString.length();
		break;

	case RegExpr:
	{
		int index = mRegExp.indexIn(string, position, QRegExp::CaretAtZero);
		if (index == position) match = mRegExp.matchedLength();
		break;
	}

	case IncludeRules:
		qDebug() << "Warning: IncludeRules left in the SyntaxDefinition after linking! :(";
		break;

	case Keyword:
	{
		foreach (QString keyword, mKeywordLink->items)
		{
			if (Tools::compareSubstring(string, keyword, position, mCaseInsensitive?Qt::CaseInsensitive:Qt::CaseSensitive))
			{
				match = keyword.length();
				break;
			}
		}
		break;
	}

	case DetectSpaces:
		while (position + match < string.length() && string.at(position + match).isSpace())
			match++;
		break;

	case Int:
		break;

	case Float:
		break;

	case HlCOct:
		break;

	case HlCHex:
		break;

	case HlCStringChar:
		break;

	case HlCChar:
		break;

	case RangeDetect:
		break;

	case LineContinue:
		break;

	case DetectIdentifier:
		break;
	}

	return match;
}





























