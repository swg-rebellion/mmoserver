#include "LogManager.h"

#include <cstdarg>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

LogManager* LogManager::mSingleton;

class LOG_ENTRY
{
public:
	LogManager::LOG_PRIORITY	mPriority;
	uint8			mChannels;
	std::string		mMessage;

	bool mContinuation;
};

LogManager::LogManager()
{
	_printLogo();

	boost::thread t(std::tr1::bind(&LogManager::_LoggerThread, this));
	mThread = boost::move(t);

	//Set the Defaults to No Logs.
	mMinPriorities[0] = 0;
	mMinPriorities[1] = 0;
	mMinPriorities[2] = 0;
}

bool LogManager::setupConsoleLogging(LOG_PRIORITY min_priority)
{
	mMinPriorities[0] = min_priority;
	return true;
}

bool LogManager::setupFileLogging(LOG_PRIORITY min_priority, std::string filename)
{
	mMinPriorities[1] = min_priority;
	FILE* tempFile = fopen(filename.c_str(), "w");

	if(!tempFile)
		return false;

	mOutputFile = tempFile;

	return true;
}

void LogManager::_LoggerThread()
{
	std::vector<LOG_ENTRY*> mTempEntries;

	while(true)
	{
		this->mEntriesMutex.lock();

		mTempEntries.reserve(mEntries.size());

		while(mEntries.size() > 0)
		{
			mTempEntries.push_back(mEntries.front());
			mEntries.pop();
		}
		this->mEntriesMutex.unlock();

		std::vector<LOG_ENTRY*>::iterator end = mTempEntries.end();

		struct tm * t;
		time_t te = time(NULL);
		t = localtime (&te);

		for(std::vector<LOG_ENTRY*>::iterator it=mTempEntries.begin(); it != end; it++)
		{
			if((*it)->mChannels & LOG_CHANNEL_CONSOLE && ((*it)->mPriority <= mMinPriorities[0]))
			{
				if(!(*it)->mContinuation)
					printf("[%d:%d:%d]<%d> ",t->tm_hour,t->tm_min,t->tm_sec, (int)(*it)->mPriority);

				printf("%s\n", (*it)->mMessage.c_str());
			}
			
			if((*it)->mChannels & LOG_CHANNEL_FILE && ((*it)->mPriority <= mMinPriorities[1]))
			{
				if(mOutputFile)
				{
					if(!(*it)->mContinuation)
						fprintf(mOutputFile, "[%d:%d:%d]<%d> ",t->tm_hour,t->tm_min,t->tm_sec, (int)(*it)->mPriority);
					
					fprintf(mOutputFile, "%s\n", (*it)->mMessage.c_str());
				}
			}

			//if((*it)->mChannels & LOG_CHANNEL_DATABASE && ((*it)->mPriority <= mMinPriorities[2]))
			//{
			//}

			delete (*it);
		}

		mTempEntries.clear();

		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
}

void	LogManager::_printLogo()
{
printf("There is Another...                                          \n");
printf("                    ______        ______    _    _   _ _   _ \n");
printf("                   / ___\\ \\      / / ___|  / \\  | \\ | | | | |\n");
printf("                   \\___ \\\\ \\ /\\ / / |  _  / _ \\ |  \\| | |_| |\n");
printf("                    ___) |\\ V  V /| |_| |/ ___ \\| |\\  |  _  |\n");
printf("                   |____/  \\_/\\_/  \\____/_/   \\_\\_| \\_|_| |_|\n");
}
	
void LogManager::log(LOG_PRIORITY priority, std::string format, ...)
{

	//Read the VA List Info
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format.c_str(), args);
	va_end(args);

	LOG_ENTRY* entry = new LOG_ENTRY();

	entry->mPriority = priority;
	entry->mChannels = LOG_CHANNEL_ALL;
	entry->mMessage	 = buffer;
	entry->mContinuation = false;

	mEntriesMutex.lock();
	mEntries.push(entry);
	mEntriesMutex.unlock();
}

void LogManager::logCont(LOG_PRIORITY priority, std::string format, ...)
{

	//Read the VA List Info
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format.c_str(), args);
	va_end(args);

	LOG_ENTRY* entry = new LOG_ENTRY();

	entry->mPriority = priority;
	entry->mChannels = LOG_CHANNEL_ALL;
	entry->mMessage	 = buffer;
	entry->mContinuation = true;

	mEntriesMutex.lock();
	mEntries.push(entry);
	mEntriesMutex.unlock();
}

void LogManager::logS(LOG_PRIORITY priority, uint8 channels, std::string format, ...)
{
	//Read the VA List Info
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format.c_str(), args);
	va_end(args);

	LOG_ENTRY* entry = new LOG_ENTRY();

	entry->mPriority = priority;
	entry->mChannels = channels;
	entry->mMessage	 = buffer;
	entry->mContinuation = false;

	mEntriesMutex.lock();
	mEntries.push(entry);
	mEntriesMutex.unlock();
}

void LogManager::logContS(LOG_PRIORITY priority, uint8 channels, std::string format, ...)
{
	//Read the VA List Info
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format.c_str(), args);
	va_end(args);

	LOG_ENTRY* entry = new LOG_ENTRY();

	entry->mPriority = priority;
	entry->mChannels = channels;
	entry->mMessage	 = buffer;
	entry->mContinuation = true;

	mEntriesMutex.lock();
	mEntries.push(entry);
	mEntriesMutex.unlock();
}