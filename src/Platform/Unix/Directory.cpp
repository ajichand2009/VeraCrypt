/*
 Derived from source code of TrueCrypt 7.1a, which is
 Copyright (c) 2008-2012 TrueCrypt Developers Association and which is governed
 by the TrueCrypt License 3.0.

 Modifications and additions to the original source code (contained in this file)
 and all other portions of this file are Copyright (c) 2013-2025 AM Crypto
 and are governed by the Apache License 2.0 the full text of which is
 contained in the file License.txt included in VeraCrypt binary and source
 code distribution packages.
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "System.h"
#include "Platform/Directory.h"
#include "Platform/Finally.h"
#include "Platform/SystemException.h"

namespace VeraCrypt
{
	static Mutex ReadDirMutex;	// readdir_r() may be unsafe on some systems

	void Directory::Create (const DirectoryPath &path)
	{
		string p = path;
		throw_sys_sub_if (mkdir (p.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == -1, p);
	}

	DirectoryPath Directory::AppendSeparator (const DirectoryPath &path)
	{
		wstring p (path);

		if (p.find_last_of (L'/') + 1 != p.size())
			return p + L'/';

		return p;
	}

	FilePathList Directory::GetFilePaths (const DirectoryPath &path, bool regularFilesOnly)
	{
		DIR *dir = opendir (string (path).c_str());
		throw_sys_sub_if (!dir, wstring (path));
		finally_do_arg (DIR*, dir, { closedir (finally_arg); });

		ScopeLock lock (ReadDirMutex);

		FilePathList files;
		struct dirent *dirEntry;
		errno = 0;
		while ((dirEntry = readdir (dir)) != nullptr)
		{
			shared_ptr <FilePath> filePath (new FilePath (string (AppendSeparator (path)) + string (dirEntry->d_name)));

			if (!regularFilesOnly || filePath->IsFile())
				files.push_back (filePath);

			errno = 0;
		}

		throw_sys_sub_if (errno != 0, wstring (path));
		return files;
	}
}
