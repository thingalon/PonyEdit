#ifndef TOOLS_H
#define TOOLS_H

#include <QString>

class Tools
{
public:
	static QString humanReadableBytes(quint64 bytes);
	static void saveServers();
	static void loadServers();
};

#endif // TOOLS_H