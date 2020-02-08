#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <streambuf>
#include <iomanip>
#include <Python.h>

#define APP_NAME "Custom PyLoader for Python 2.7"
const char *defaultfile = "a.py";
const char *defaultfile2 = "b.py";

typedef void(*PTR_PyEval_InitThreads)();
PTR_PyEval_InitThreads PyEvalInitThreads = NULL;

typedef enum PyGILStateSTATE(*PTR_PyGILState_Ensure)(void);
PTR_PyGILState_Ensure PyGILStateEnsure = NULL;

typedef void(*PTR_PyGILState_Release)(enum PyGILStateSTATE);
PTR_PyGILState_Release PyGILStateRelease = NULL;

typedef int(*PyRunSimpleFile)(FILE *fp, const char *filename);
PyRunSimpleFile python_runsimplefile = NULL;

typedef FILE*(*PyFileAsFile)(PyObject *p);
PyFileAsFile python_asfile = NULL;

typedef PyObject*(*PyFileFromString)(char *filename, char *mode);
PyFileFromString python_fromstring = NULL;

HMODULE g_PythonDLL = NULL;

LPDWORD g_ThreadID = NULL;

void LoadFile(char* fileName) {
	PyObject* PyFileObject = python_fromstring(fileName, (char*)"r+");
	if (PyFileObject == NULL) {
		MessageBoxA(NULL, "could not find file", "Error",
			MB_ICONERROR);
	}

	FILE* f = python_asfile(PyFileObject);
	python_runsimplefile(f, fileName);
}

DWORD WINAPI LoadPythonFiles(LPVOID lpParam) {
	MessageBoxA(NULL, "loading start", "Error",
		MB_ICONERROR);

	PyGILStateSTATE state = PyGILStateEnsure();

	LoadFile((char*)defaultfile);
	LoadFile((char*)defaultfile2);

	PyGILStateRelease(state);

	MessageBoxA(NULL, "loading end", "Error",
		MB_ICONERROR);

	return TRUE;
}

BOOL LoadPythonLib()
{
	if (!(g_PythonDLL = GetModuleHandleA("python27.dll"))) {
		MessageBoxA(NULL, "Could not load python27.dll!", "Error",
			MB_ICONERROR);
		return FALSE;
	}

	PyEvalInitThreads = (PTR_PyEval_InitThreads)GetProcAddress(
		g_PythonDLL, "PyEval_InitThreads");
	if (!PyEvalInitThreads) {
		MessageBoxA(NULL,
			"An error occured! Could not find PyEval_InitThreads!",
			"Error", MB_ICONERROR);
		return FALSE;
	}

	PyEvalInitThreads();

	PyGILStateEnsure = (PTR_PyGILState_Ensure)GetProcAddress(
		g_PythonDLL, "PyGILState_Ensure");
	if (!PyGILStateEnsure) {
		MessageBoxA(NULL,
			"An error occured! Could not find PyGILState_Ensure!",
			"Error", MB_ICONERROR);
		return FALSE;
	}

	PyGILStateRelease = (PTR_PyGILState_Release)GetProcAddress(
		g_PythonDLL, "PyGILState_Release");
	if (!PyGILStateRelease) {
		MessageBoxA(NULL,
			"An error occured! Could not find PyGILState_Release!",
			"Error", MB_ICONERROR);
		return FALSE;
	}

	python_runsimplefile = (PyRunSimpleFile)GetProcAddress(g_PythonDLL, "PyRun_SimpleFile");
	if (NULL == python_runsimplefile)
	{
		MessageBoxA(NULL,
			"An error occured! Could not find PyRun_SimpleFile!",
			"Error", MB_ICONERROR);
		return FALSE;
	}

	python_fromstring = (PyFileFromString)GetProcAddress(g_PythonDLL, "PyFile_FromString");
	if (NULL == python_fromstring)
	{
		MessageBoxA(NULL,
			"An error occured! Could not find python_fromstring!",
			"Error", MB_ICONERROR);
		return FALSE;
	}

	python_asfile = (PyFileAsFile)GetProcAddress(g_PythonDLL, "PyFile_AsFile");
	if (NULL == python_asfile)
	{
		MessageBoxA(NULL,
			"An error occured! Could not find python_asfile!",
			"Error", MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls((HMODULE)hModule);
		if (!LoadPythonLib())
		{
			return FALSE;
		}
		CreateThread(NULL, 0, LoadPythonFiles, 0, 0, g_ThreadID);
	}
	return TRUE;
}