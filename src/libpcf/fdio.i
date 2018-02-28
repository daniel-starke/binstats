/**
 * @file fdio.i
 * @author Daniel Starke
 * @copyright Copyright 2016-2018 Daniel Starke
 * @see fdios.h
 * @see fdious.h
 * @date 2016-08-17
 * @version 2018-02-27
 * @internal This file is never used or compiled directly but only included.
 * @remarks Define CHAR_T to the character type before including this file.
 * @remarks See FPOPEN_FUNC() and FPCLOSE_FUNC() for further notes.
 * @see https://blogs.msdn.microsoft.com/oldnewthing/20111216-00/?p=8873/
 */
#define _POSIX_C_SOURCE 200809L
#define __XOPEN_2K8 1
#include <stdio.h>
#include <string.h>
#include <libpcf/target.h>

#ifdef PCF_IS_WIN
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <winbase.h>
#else /* ! PCF_IS_WIN */
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#ifdef FPOPEN_UNICODE
#include <wchar.h>
#endif
#include <sys/wait.h>
#endif /* PCF_IS_WIN */


#ifndef READ_PIPE
#define READ_PIPE 0
#endif
#ifndef WRITE_PIPE
#define WRITE_PIPE 1
#endif


/**
 * @fn tFdioPHandle * FPOPEN_FUNC(const CHAR_T ** shell, const CHAR_T * command, FILE * input, const char * mode)
 * Executes the passed program and opens its stdin or stdout for reading
 * or writing. The difference to popen is, that the command expects a shell
 * which shall be used for execution.
 * The first argument of 'shell' is the program path and any other following except NULL are the
 * arguments that shall be passed to shell for executing the given command.
 *
 * @param[in] shellPath - path the shell binary
 * @param[in] shell - execute the given command with this shell (null terminated array)
 * @param[in] command - command to execute (optional)
 * @param[in] input - use this for the standard input of the new process (optional)
 * @param[in] mode - open mode, see tFdioPMode
 * @return NULL on error or the handle to the child process
 * @remarks Define FPOPEN_FUNC for name of the function.
 * @remarks Define FPOPEN_UNICODE for Unicode Windows build.
 * @remarks Windows implementation is based on https://msdn.microsoft.com/en-us/library/17w5ykft(v=vs.85).aspx
 * except if FDIO_RAW_CMDLINE is passed, which forces the function to pass the arguments separated by space.
 */


/**
 * @fn int FPCLOSE_FUNC(tFdioPHandle * fd)
 * Closes the read/write file descriptor of the passed handle and waits until the child
 * process terminates. Returns the exit code of the child process.
 *
 * @param[in,out] fd - tFdioPHandle pointer to the FPOPEN handle
 * @return -1 on error or the exit code of the child process
 * @remarks Define FPCLOSE_FUNC for name of the function.
 */


#ifdef FPOPEN_UNICODE
#define CHAR_T wchar_t
#ifndef TCHAR_STLEN
#define TCHAR_STLEN wcslen
#endif /* TCHAR_STLEN */
#else /* ! FPOPEN_UNICODE */
#define CHAR_T char
#ifndef TCHAR_STLEN
#define TCHAR_STLEN strlen
#endif /* TCHAR_STLEN */
#endif /* FPOPEN_UNICODE */


#ifdef PCF_IS_WIN
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA 0x0600
#endif


/* Defined in fdio.c. */
extern volatile HANDLE _LIBPCF_CREATEPROCESS_MUTEX;


#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
/* It is not possible to inherit FILE_TYPE_CHAR handles to a child process via
 * PROC_THREAD_ATTRIBUTE_HANDLE_LIST.
 */
static int isInheritableHandle(HANDLE handle) {
	if ( ! handle ) return 0;
	if (handle == INVALID_HANDLE_VALUE) return 0;
	DWORD type = GetFileType(handle);
	return type == FILE_TYPE_DISK || type == FILE_TYPE_PIPE;
}
#endif /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */


tFdioPHandle * FPOPEN_FUNC(const CHAR_T * shellPath, const CHAR_T ** shell, const CHAR_T * command, FILE * input, const tFdioPMode mode) {
	HANDLE pStandardInput[2];
	HANDLE pStandardOutput[2];
	HANDLE pStandardError[2];
	int hasPStdIn, hasPStdOut, hasPStdErr;
	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#ifdef FPOPEN_UNICODE
	STARTUPINFOW si;
	STARTUPINFOW * siPtr;
#else /* ! FPOPEN_UNICODE */
	STARTUPINFOA si;
	STARTUPINFOA * siPtr;
#endif /* FPOPEN_UNICODE */
#else /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */
	HANDLE pInheritHandles[3];
#ifdef FPOPEN_UNICODE
	STARTUPINFOEXW si;
	STARTUPINFOW * siPtr;
#else /* ! FPOPEN_UNICODE */
	STARTUPINFOEXA si;
	STARTUPINFOA * siPtr;
#endif /* FPOPEN_UNICODE */
	LPPROC_THREAD_ATTRIBUTE_LIST al;
	SIZE_T alSize;
	int hasAl;
#endif /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */
	DWORD cf; /* creation flags */
	BOOL bSuccess = FALSE;
	tFdioPHandle * fd;
	const CHAR_T * cPtr;
	CHAR_T * cmdLine, * clPtr;
	size_t numArgs;
	const CHAR_T * arg, ** args;
	int * hasSpaces;
	size_t cmdLineLen, escCount;
	int i, flags;
	int vistaOrNewer;

	if (shellPath == NULL || shell == NULL || *shell == NULL) return NULL;
	/* cannot access the standard input of the sub process and use a different file descriptor
	   as standard input for it */
	if ((((int)mode) & FDIO_USE_STDIN) != 0 && input != NULL) return NULL;
	hasPStdIn = 0;
	hasPStdOut = 0;
	hasPStdErr = 0;
	fd = NULL;
	cmdLine = NULL;
	hasSpaces = NULL;
	cf = 0;
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
	al = NULL;
	hasAl = 0;
	vistaOrNewer = (LOBYTE(LOWORD(GetVersion())) >= 6) ? 1 : 0;
#else /* _WIN32_WINNT < _WIN32_WINNT_VISTA */
	vistaOrNewer = 0;
#endif /* _WIN32_WINNT < _WIN32_WINNT_VISTA */
	
	if ((((int)mode) & FDIO_RAW_CMDLINE) != 0) {
		/* check the needed size for the command-line string */
		cmdLineLen = 1; /* null-termination */
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			cmdLineLen += TCHAR_STLEN(arg);
			if (arg == command) break;
			cmdLineLen++; /* argument separating space */
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		/* create the command-line */
		cmdLine = (CHAR_T *)malloc(sizeof(CHAR_T) * cmdLineLen);
		if (cmdLine == NULL) {
			goto onerror;
		}
		clPtr = cmdLine;
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			cmdLineLen = TCHAR_STLEN(arg);
			memcpy(clPtr, arg, cmdLineLen * sizeof(CHAR_T));
			clPtr += cmdLineLen;
			if (arg == command) break;
			/* spaces between two arguments */
			*clPtr = ' ';
			clPtr++;
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		*clPtr = 0;
	} else {
		/* calculate passed number of arguments */
		numArgs = 1;
		args = shell;
		for (arg = *args; arg != NULL; args++, arg = *args, numArgs++);
		/* create array to save whether a arguments needs quoting or not */
		hasSpaces = (int *)malloc(sizeof(int) * numArgs);
		if (hasSpaces == NULL) goto onerror;
		/* check the needed size for the command-line string */
		/* see https://msdn.microsoft.com/en-us/library/17w5ykft(v=vs.85).aspx */
		cmdLineLen = 3; /* first quote, last quote and null-termination */
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			hasSpaces[i] = 0;
			escCount = 0;
			for (cPtr = arg; *cPtr != 0; cPtr++) {
				switch (*cPtr) {
				case '\\':
					escCount++;
					break;
				case '"':
					if (escCount > 0) {
						cmdLineLen = cmdLineLen + escCount + 1;
						escCount = 0;
					} else {
						cmdLineLen++;
					}
					break;
				default:
					if (*cPtr == ' ' || *cPtr == '\t') hasSpaces[i] = 1;
					escCount = 0;
					break;
				}
				cmdLineLen++;
			}
			cmdLineLen += escCount; /* handle trailing escape characters */
			if (hasSpaces[i] != 0) cmdLineLen += 2;
			if (arg == command) break;
			cmdLineLen++; /* argument separating space */
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		/* create the command-line */
		cmdLine = (CHAR_T *)malloc(sizeof(CHAR_T) * cmdLineLen);
		if (cmdLine == NULL) {
			goto onerror;
		}
		clPtr = cmdLine;
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			if (hasSpaces[i] != 0) {
				/* quote of the beginning of an argument */
				*clPtr = '"';
				clPtr++;
			}
			escCount = 0;
			for (cPtr = arg; *cPtr != 0; cPtr++) {
				switch (*cPtr) {
				case '\\':
					escCount++;
					break;
				case '"':
					if (escCount > 0) {
						for (; escCount > 0; escCount--) {
							*clPtr = '\\';
							clPtr++;
						}
						escCount = 0;
					}
					*clPtr = '\\';
					clPtr++;
					break;
				default:
					escCount = 0;
					break;
				}
				*clPtr = *cPtr;
				clPtr++;
			}
			for (; escCount > 0; escCount--) {
				/* handle trailing escape characters */
				*clPtr = '\\';
				clPtr++;
			}
			if (hasSpaces[i] != 0) {
				/* quote of the end of an argument */
				*clPtr = '"';
				clPtr++;
			}
			if (arg == command) break;
			/* spaces between two arguments */
			*clPtr = ' ';
			clPtr++;
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		*clPtr = 0;
	}
	
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	if ( ! vistaOrNewer ) {
		/* We need to ensure that only one thread runs this function at the time when dealing
		 * with Windows versions earlier than Vista. This is a helper function for this. */
		if (_LIBPCF_CREATEPROCESS_MUTEX == NULL) {
			HANDLE p = CreateMutex(NULL, FALSE, NULL);
			if (InterlockedCompareExchangePointer((PVOID *)(&_LIBPCF_CREATEPROCESS_MUTEX), (PVOID)p, NULL) != NULL) {
				CloseHandle(p);
			}
		}
		if (WaitForSingleObject(_LIBPCF_CREATEPROCESS_MUTEX, INFINITE) == WAIT_FAILED) goto onerror;
	}
	
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		if ( ! CreatePipe(&(pStandardInput[READ_PIPE]), &(pStandardInput[WRITE_PIPE]), &sa, 0 /* default buffer size */) ) {
			goto onerror;
		}
		hasPStdIn = 1;
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		if ( ! CreatePipe(&(pStandardOutput[READ_PIPE]), &(pStandardOutput[WRITE_PIPE]), &sa, 0 /* default buffer size */) ) {
			goto onerror;
		}
		hasPStdOut = 1;
	}
	
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		if ( ! CreatePipe(&(pStandardError[READ_PIPE]), &(pStandardError[WRITE_PIPE]), &sa, 0 /* default buffer size */) ) {
			goto onerror;
		}
		hasPStdErr = 1;
	}
	
	if ((fd = (tFdioPHandle *)malloc(sizeof(tFdioPHandle))) == NULL) {
		goto onerror;
	}
	
	/* disable inheritance for parent handles */
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		if ( ! SetHandleInformation(pStandardInput[WRITE_PIPE], HANDLE_FLAG_INHERIT, 0) ) {
			goto onerror;
		}
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		if ( ! SetHandleInformation(pStandardOutput[READ_PIPE], HANDLE_FLAG_INHERIT, 0) ) {
			goto onerror;
		}
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		if ( ! SetHandleInformation(pStandardError[READ_PIPE], HANDLE_FLAG_INHERIT, 0) ) {
			goto onerror;
		}
	}
	
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
	si.cb = sizeof(si);
	siPtr = &si;
#else /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */
	si.StartupInfo.cb = sizeof(si);
	siPtr = &(si.StartupInfo);
	
	si.StartupInfo.cb = sizeof(si);
	
	/* use this to avoid race conditions when passing handles to the child process (needs >=Win Vista) */
	if ( vistaOrNewer ) {
		alSize = 0;
		bSuccess = InitializeProcThreadAttributeList(NULL, 1, 0, &alSize) || GetLastError() == ERROR_INSUFFICIENT_BUFFER;
		if ( ! bSuccess ) goto onerror;
		
		al = (LPPROC_THREAD_ATTRIBUTE_LIST)(HeapAlloc(GetProcessHeap(), 0, alSize));
		if (al == NULL) goto onerror;
		
		bSuccess = InitializeProcThreadAttributeList(al, 1, 0, &alSize);
		if ( ! bSuccess ) goto onerror;
		
		hasAl = 1;
	}
#endif /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */
	
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		siPtr->hStdInput = pStandardInput[READ_PIPE];
	} else if (input != NULL) {
		siPtr->hStdInput = (HANDLE)_get_osfhandle(fileno(input));
	} else {
		siPtr->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		if (siPtr->hStdInput == INVALID_HANDLE_VALUE) goto onerror;
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		siPtr->hStdOutput = pStandardOutput[WRITE_PIPE];
	} else {
		siPtr->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		if (siPtr->hStdOutput == INVALID_HANDLE_VALUE) goto onerror;
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0) {
		if ((((int)mode) & FDIO_COMBINE) == 0) {
			siPtr->hStdError = pStandardError[WRITE_PIPE];
		} else {
			siPtr->hStdError = siPtr->hStdOutput;
		}
	} else {
		if ((((int)mode) & FDIO_COMBINE) == 0) {
			siPtr->hStdError = GetStdHandle(STD_ERROR_HANDLE);
			if (siPtr->hStdError == INVALID_HANDLE_VALUE) goto onerror;
		} else {
			siPtr->hStdError = siPtr->hStdOutput;
		}
	}
	siPtr->dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siPtr->wShowWindow = SW_HIDE;
	
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
	if ( vistaOrNewer ) {
		i = 0;
		if ( isInheritableHandle(siPtr->hStdInput) ) pInheritHandles[i++] = siPtr->hStdInput;
		if ( isInheritableHandle(siPtr->hStdOutput) ) pInheritHandles[i++] = siPtr->hStdOutput;
		if (isInheritableHandle(siPtr->hStdError) && siPtr->hStdOutput != siPtr->hStdError) {
			pInheritHandles[i++] = siPtr->hStdError;
		}
		
		if (i > 0) {
			bSuccess = UpdateProcThreadAttribute(
				al,
				0,
				PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
				pInheritHandles,
				(SIZE_T)((SIZE_T)i * sizeof(HANDLE)),
				NULL,
				NULL
			);
			
			if ( ! bSuccess ) goto onerror;
			
			si.lpAttributeList = al;
			cf = EXTENDED_STARTUPINFO_PRESENT;
		}
	}
#endif
	
#ifdef FPOPEN_UNICODE
	bSuccess = CreateProcessW(shellPath, cmdLine, NULL, NULL, TRUE, cf, NULL, NULL, siPtr, &pi);
#else /* ! FPOPEN_UNICODE */
	bSuccess = CreateProcessA(shellPath, cmdLine, NULL, NULL, TRUE, cf, NULL, NULL, siPtr, &pi);
#endif /* FPOPEN_UNICODE */

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
	if (hasAl != 0) {
		DeleteProcThreadAttributeList(al);
		hasAl = 0;
	}
	if (al != NULL) {
		HeapFree(GetProcessHeap(), 0, al);
		al = NULL;
	}
#endif /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */
	
	free(hasSpaces);
	hasSpaces = NULL;
	free(cmdLine);
	cmdLine = NULL;
	if ( ! bSuccess ) {
		goto onerror;
	}
	/* close pipe handles after child process inherited them */
	/* see https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx */
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		CloseHandle(siPtr->hStdInput);
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		CloseHandle(siPtr->hStdOutput);
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		CloseHandle(siPtr->hStdError);
	}
	
	CloseHandle(pi.hThread);
	flags = 0;
	if ((((int)mode) & FDIO_BINARY_PIPE) == 0) flags |= O_TEXT;
	fd->pid = pi.hProcess;
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		fd->in = fdopen(
			open_osfhandle((intptr_t)pStandardInput[WRITE_PIPE], flags),
			(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "wb" : "w"
		);
	} else {
		fd->in = NULL;
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		fd->out = fdopen(
			open_osfhandle((intptr_t)pStandardOutput[READ_PIPE], flags | O_RDONLY),
			(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
		);
	} else {
		fd->out = NULL;
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		fd->err = fdopen(
			open_osfhandle((intptr_t)pStandardError[READ_PIPE], flags | O_RDONLY),
			(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
		);
	} else {
		fd->err = NULL;
	}
	
	if ( ! vistaOrNewer ) {
		ReleaseMutex(_LIBPCF_CREATEPROCESS_MUTEX);
	}
	
	return fd;
onerror:
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
	if (hasAl != 0) DeleteProcThreadAttributeList(al);
	if (al != NULL) HeapFree(GetProcessHeap(), 0, al);
#endif /* _WIN32_WINNT >= _WIN32_WINNT_VISTA */
	if (hasPStdIn != 0) {
		CloseHandle(pStandardInput[READ_PIPE]);
		CloseHandle(pStandardInput[WRITE_PIPE]);
	}
	if (hasPStdOut != 0) {
		CloseHandle(pStandardOutput[READ_PIPE]);
		CloseHandle(pStandardOutput[WRITE_PIPE]);
	}
	if (hasPStdErr != 0) {
		CloseHandle(pStandardError[READ_PIPE]);
		CloseHandle(pStandardError[WRITE_PIPE]);
	}
	if ( ! vistaOrNewer ) {
		ReleaseMutex(_LIBPCF_CREATEPROCESS_MUTEX);
	}
	if (hasSpaces != NULL) free(hasSpaces);
	if (fd != NULL) free(fd);
	if (cmdLine != NULL) free(cmdLine);
	return NULL;
}


int FPCLOSE_FUNC(tFdioPHandle * fd) {
	HANDLE pid;
	int exitCode;
	DWORD dwExitCode;
	if (fd == NULL) return -1;
	pid = fd->pid;
	
	if (fd->in != NULL) fclose(fd->in);
	
	exitCode = -1;
	if (WaitForSingleObject(pid, INFINITE) != WAIT_FAILED) {
		if ( GetExitCodeProcess(pid, &dwExitCode) ) {
			exitCode = (int)dwExitCode;
		}
	}
	
	/* standard outputs need to be closed last or the child process
	 * may fail with an error code while trying to write to them */
	if (fd->out != NULL) fclose(fd->out);
	if (fd->err != NULL) fclose(fd->err);
	free(fd);
	
	CloseHandle(pid);
	return exitCode;
}
#else /* ! PCF_IS_WIN */
#ifndef NSIG
#define NSIG 32
#endif


/* Defined in fdio.c. */
extern int fdio_closeNonDefFds();


tFdioPHandle * FPOPEN_FUNC(const CHAR_T * shellPath, const char ** shell, const char * command, FILE * input, const tFdioPMode mode) {
	int pStandardInput[2];
	int pStandardOutput[2];
	int pStandardError[2];
	int hasPStdIn, hasPStdOut, hasPStdErr;
	sigset_t oldMask, newMask;
	struct sigaction sigAction;
	int hasOldMask;
	tFdioPHandle * fd;
	size_t numArgs, strLen, totalStrLen;
	const CHAR_T ** args;
	CHAR_T * out;
	CHAR_T ** argv;
	int i;

	if (shellPath == NULL || shell == NULL || *shell == NULL) return NULL;
	/* cannot access the standard input of the sub process and use a different file descriptor
	   as standard input for it */
	if ((((int)mode) & FDIO_USE_STDIN) != 0 && input != NULL) return NULL;
	
	hasPStdIn = 0;
	hasPStdOut = 0;
	hasPStdErr = 0;
	hasOldMask = 0;
	fd = NULL;
	argv = NULL;
	
	/* calculate passed number of arguments and final memory size */
	numArgs = 1;
	totalStrLen = 0;
	if (command != NULL) {
		numArgs++;
		totalStrLen += TCHAR_STLEN(command) + 1;
	}
	for (args = shell; *args != NULL; args++, numArgs++) {
		totalStrLen += TCHAR_STLEN(*args) + 1;
	}
	
	/* build new command array */
	argv = (CHAR_T **)malloc((sizeof(CHAR_T *) * numArgs) + (sizeof(CHAR_T) * totalStrLen));
	if (argv == NULL) goto onerror;
	out = (CHAR_T *)(&(argv[numArgs]));
	for (args = shell, i = 0; *args != NULL; args++, i++) {
		strLen = TCHAR_STLEN(*args) + 1;
		memcpy(out, *args, sizeof(CHAR_T) * strLen);
		argv[i] = out;
		out += strLen;
	}
	if (command != NULL) {
		strLen = TCHAR_STLEN(command) + 1;
		memcpy(out, command, sizeof(CHAR_T) * strLen);
		argv[i] = out;
		i++;
	}
	argv[i] = NULL;
	
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		if (pipe(pStandardInput) != 0) goto onerror;
		hasPStdIn = 1;
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		if (pipe(pStandardOutput) != 0) goto onerror;
		hasPStdOut = 1;
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		if (pipe(pStandardError) != 0) goto onerror;
		hasPStdErr = 1;
	}

	if ((fd = (tFdioPHandle *)malloc(sizeof(tFdioPHandle))) == NULL) goto onerror;
	
	/* Temporary disable signal handling for calling thread to avoid unexpected signals between
	 * fork() and execvp(). */
	sigfillset(&newMask);
	if (pthread_sigmask(SIG_SETMASK, &newMask, &oldMask) != 0) goto onerror;
	hasOldMask = 1;
	
	fd->pid = fork();

	switch (fd->pid) {
	case -1: /* fork failed */
		goto onerror;
		break;
	case 0: /* child */
		free(fd);
		/* clear out signal handlers to avoid unexpected events */
		sigAction.sa_handler = SIG_DFL;
		sigAction.sa_flags = 0;
		sigemptyset(&sigAction.sa_mask);
		for (i = 1; i < NSIG; i++) sigaction(i, &sigAction, NULL);
		if (pthread_sigmask(SIG_SETMASK, &oldMask, NULL) < 0) goto onerror;
	
		/* set stdin/stdout/stderr accordingly */
		if ((((int)mode) & FDIO_USE_STDIN) != 0) {
			dup2(pStandardInput[READ_PIPE], STDIN_FILENO);
		} else if (input != NULL) {
			dup2(fileno(input), STDIN_FILENO);
		}
		if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
			dup2(pStandardOutput[WRITE_PIPE], STDOUT_FILENO);
		}
		if ((((int)mode) & FDIO_USE_STDERR) != 0) {
			if ((((int)mode) & FDIO_COMBINE) == 0) {
				dup2(pStandardError[WRITE_PIPE], STDERR_FILENO);
			} else {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			}
		} else {
			if ((((int)mode) & FDIO_COMBINE) != 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			}
		}
		
		/* close other pipe fds */
		if (hasPStdIn != 0) close(pStandardInput[WRITE_PIPE]);
		if (hasPStdOut != 0) close(pStandardOutput[READ_PIPE]);
		if (hasPStdErr != 0) close(pStandardError[READ_PIPE]);
		
		/* close all unwanted file descriptors inherited from the parent process */
		if (fdio_closeNonDefFds() != 1) exit(1);
		
		/* can change to any exec* function family */
		/* see why argv cannot be const char **: http://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html#tag_16_111 */
		execvp(shellPath, argv);
		exit(1); /* something went wrong */
		
		break;
	default: /* parent */
		/* restore original signal handler */
		if (hasOldMask != 0) {
			pthread_sigmask(SIG_SETMASK, &oldMask, NULL);
		}
		
		/* close other pipe fds */
		if (hasPStdIn != 0) close(pStandardInput[READ_PIPE]);
		if (hasPStdOut != 0) close(pStandardOutput[WRITE_PIPE]);
		if (hasPStdErr != 0) close(pStandardError[WRITE_PIPE]);
		
		if ((((int)mode) & FDIO_USE_STDIN) != 0) {
			if ((fd->in = fdopen(
				pStandardInput[WRITE_PIPE],
				(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "wb" : "w"
			)) == NULL) {
				close(pStandardInput[WRITE_PIPE]);
			}
		} else {
			fd->in = NULL;
		}
		if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
			if ((fd->out = fdopen(
				pStandardOutput[READ_PIPE],
				(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
			)) == NULL) {
				close(pStandardOutput[READ_PIPE]);
			}
		} else {
			fd->out = NULL;
		}
		if ((((int)mode) & FDIO_USE_STDERR) != 0) {
			if ((fd->err = fdopen(
				pStandardError[READ_PIPE],
				(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
			)) == NULL) {
				close(pStandardError[READ_PIPE]);
			}
		} else {
			fd->err = NULL;
		}
		if (argv != NULL) free(argv);
		break;
	}
	
	return fd;
onerror:
	/* restore original signal handler */
	if (hasOldMask != 0) {
		pthread_sigmask(SIG_SETMASK, &oldMask, NULL);
	}
	if (hasPStdIn != 0) {
		close(pStandardInput[READ_PIPE]);
		close(pStandardInput[WRITE_PIPE]);
	}
	if (hasPStdOut != 0) {
		close(pStandardOutput[READ_PIPE]);
		close(pStandardOutput[WRITE_PIPE]);
	}
	if (hasPStdErr != 0) {
		close(pStandardError[READ_PIPE]);
		close(pStandardError[WRITE_PIPE]);
	}
	if (fd != NULL) free(fd);
	if (argv != NULL) free(argv);
	return NULL;
}


int FPCLOSE_FUNC(tFdioPHandle * fd) {
	int status, result;
	if (fd == NULL) return -1;
	
	if (fd->in != NULL) fclose(fd->in);
	
	result = -1;
	do {
		errno = 0;
		if (waitpid(fd->pid, &status, 0) >= 0) break;
	} while (errno == EINTR);
	
	if ( WIFEXITED(status) ) {
		result = WEXITSTATUS(status);
	}
	
	/* standard outputs need to be closed last or the child process
	 * may fail with an error while trying to write to them */
	if (fd->out != NULL) fclose(fd->out);
	if (fd->err != NULL) fclose(fd->err);
	free(fd);
	
	return result;
}
#endif /* PCF_IS_WIN */
