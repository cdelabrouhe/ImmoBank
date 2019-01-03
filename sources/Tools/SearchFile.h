#pragma once

//#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ImmoBank
{
	struct SearchFile
	{
		typedef std::vector<std::string> FileNameArray;
		FileNameArray files;

		FileNameArray::iterator begin()
		{
			return files.begin();
		}

		FileNameArray::iterator end()
		{
			return files.end();
		}

		int count() const
		{
			return (int)files.size();
		}

		std::string operator[](int index)
		{
			return files[index];
		}

		void ListFoldersInDir(const std::string &_searchPath, std::vector<std::string> & _list)
		{
			WIN32_FIND_DATAA wfd;

			std::string findwhat = _searchPath + "/*";  // directory

			HANDLE hf = FindFirstFileA(findwhat.c_str(), &wfd);
			while (hf != INVALID_HANDLE_VALUE)
			{
				if (wfd.cFileName[0] != '.' && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					const std::string found = _searchPath + "/" + wfd.cFileName;

					_list.push_back(found);
				}

				if (!FindNextFileA(hf, &wfd))
				{
					FindClose(hf);
					hf = INVALID_HANDLE_VALUE;
				}
			}
		}


		void operator()(const std::string &path, const std::string &pattern, const std::string & _searchPath)
		{
			WIN32_FIND_DATAA wfd;
			HANDLE hf;
			std::string findwhat;
			std::vector<std::string> dir;

			const std::string localPath = path.size() > 0 ? path + "/" : "";
			const std::string findPath = _searchPath + "/" + localPath;

			findwhat = findPath + "*";  // directory

			hf = FindFirstFileA(findwhat.c_str(), &wfd);
			while (hf != INVALID_HANDLE_VALUE)
			{
				if (wfd.cFileName[0] != '.' && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					const std::string found = localPath + wfd.cFileName;

					dir.push_back(found);
				}

				if (!FindNextFileA(hf, &wfd))
				{
					FindClose(hf);
					hf = INVALID_HANDLE_VALUE;
				}
			}

			findwhat = findPath + pattern;  // files

			hf = FindFirstFileA(findwhat.c_str(), &wfd);
			while (hf != INVALID_HANDLE_VALUE)
			{
				if (wfd.cFileName[0] != '.' && !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					const std::string found = localPath + wfd.cFileName;

					files.push_back(found);
				}

				if (!FindNextFileA(hf, &wfd))
				{
					FindClose(hf);
					hf = INVALID_HANDLE_VALUE;
				}
			}

			// continue with directories
			for (std::vector<std::string>::iterator it = dir.begin(); it != dir.end(); ++it)
				this->operator()(*it, pattern, _searchPath);
		}
	};
}