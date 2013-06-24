#include "history.h"
#include "Python.h"
#include <string>
#include <iostream>
#include <exception>
#include <stdio.h>
#include <stdarg.h>

PythonInterface::PythonInterface(std::string modulePath) {
	PythonInterface::modulePath = modulePath;
	if (!Py_IsInitialized()) {
		Py_Initialize();
		std::cout << "Python Initialized" << std::endl;
	}
}

PythonInterface::~PythonInterface() {

	for (std::map<std::string, PyObject*>::iterator ii =
			pObjectCache.begin(); ii != pObjectCache.end(); ++ii) {
		Py_DECREF((*ii).second);
	}

	Py_Finalize();
}

PyObject* PythonInterface::getClassObject(std::string moduleName, std::string className) {
	if (!modulePath.empty()) {

		PyRun_SimpleString("import sys");
		PyRun_SimpleString(
				("sys.path.append(\"" + std::string(modulePath) + "\")").c_str());
	}

	std::string modName = moduleName;
	PyObject *pName = PyString_FromString(modName.c_str());
	PyObject *pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	char* fileName = PyModule_GetFilename(pModule);
	char* argv[1] = { fileName };
	PySys_SetArgvEx(1, argv, 0);

	PyObject *pDict = PyModule_GetDict(pModule);
	Py_DECREF(pModule);

	PyObject *pClass = PyDict_GetItemString(pDict, className.c_str());
	Py_DECREF(pDict);

	return pClass;
}

PyObject* PythonInterface::getClassInstance(std::string moduleName, std::string className, PyObject *pClassArgs) {
	if (pObjectCache.find(moduleName + className) == pObjectCache.end()) {
		PyObject *pClass = getClassObject(moduleName, className);

		if (pClassArgs != NULL && !PyTuple_Check(pClassArgs)) {
			PyObject *temp = pClassArgs;
			pClassArgs = PyTuple_Pack(1, temp);
			Py_DECREF(temp);
		}

		pObjectCache[moduleName + className] = PyObject_CallObject(pClass, pClassArgs);

		Py_DECREF(pClass);
		Py_XDECREF(pClassArgs);

		std::cout << moduleName << " Initialized" << std::endl;
	}

	return pObjectCache[moduleName + className];
}

PyObject* PythonInterface::callMethod(PyObject* pInstance, std::string methodName) {
	return callMethod(pInstance, methodName, "", 0);
}

PyObject* PythonInterface::callMethod(PyObject* pInstance, std::string methodName,
		std::string value) {
	return callMethod(pInstance, methodName, "(s)", strdup(value.c_str()));
}
