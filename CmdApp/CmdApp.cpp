// CmdApp.cpp
// FS, 09-07-2007

#define DLLAPI_FSUTILS_CMDAPP __declspec(dllexport)
//#define showVal(val) std::cout << #val " = " << (val) << std::endl

#include "CmdApp_internal.h"

using namespace FSUtils;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;
	
	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}


CmdApp::CmdApp(std::string programName, std::string version): \
	m_programName(programName), m_programVersion(version), m_stopExecution(false), \
	m_hasFinalArg(false), m_version(false), m_help(false)
{
	CmdArg uArgVersion(CmdArgType::Boolean, &m_version, "V", "version", false);
	CmdArg uArgHelp(CmdArgType::Boolean, &m_help, "h", "help", false);

	uArgVersion.desc = "display the version and exit";
	uArgHelp.desc = "print this help";

	// Version should be the first one, help the second one
	this->bindArg(uArgVersion);
	this->bindArg(uArgHelp);
}


CmdApp::~CmdApp()
{
}


void CmdApp::setDescription(const std::string &desc)
{
	m_desc = desc;

	return;
}


void CmdApp::bindArg(const CmdArg &arg)
{
	// Check if any of the forms is a duplicate
	if(arg.shortForm != "" && std::find_if(m_vecArgs.begin(), m_vecArgs.end(),
		std::bind1st(FindArg_short(), "-" + arg.shortForm.substr(0, 1))) != m_vecArgs.end())
	{
		throw DuplicateShortForm(arg.shortForm);
	}

	if(std::find_if(m_vecArgs.begin(), m_vecArgs.end(),
		std::bind1st(FindArg_long(), "--" + arg.longForm + "=")) != m_vecArgs.end())
	{
		throw DuplicateLongForm(arg.longForm);
	}


	m_vecArgs.push_back(arg);
	CmdArg &lastArg = m_vecArgs.back();
	
	if(lastArg.shortForm != "")
		lastArg.shortForm = "-" + lastArg.shortForm.substr(0, 1);
	
	if(lastArg.argType == CmdArgType::Boolean)
		lastArg.longForm = "--" + lastArg.longForm;
	else
		lastArg.longForm = "--" + lastArg.longForm + "=";

	return;
}


void CmdApp::setFinalArg(void *var, CmdArgType type)
{
	m_finalArg.var = var;
	m_finalArg.argType = type;
	m_hasFinalArg = true;

	return;
}


void CmdApp::parseArgs(int argc, char *argv[], bool ignoreUnknown)
{
	// Reset CmdArgs
	for(std::vector<CmdArg>::iterator iterArg = m_vecArgs.begin(); \
		iterArg != m_vecArgs.end(); ++iterArg)
	{
		iterArg->isSet = false;
	}


	// Parse CmdArgs
	int endArgs = (m_hasFinalArg) ? argc - 1 : argc;
	for(int i = 1; i < endArgs; ++i)
	{
		std::vector<CmdArg>::iterator iterShort = std::find_if(
			m_vecArgs.begin(), m_vecArgs.end(), std::bind1st(FindArg_short(), argv[i]));
		std::vector<CmdArg>::iterator iterLong = std::find_if(
			m_vecArgs.begin(), m_vecArgs.end(), std::bind1st(FindArg_long(), argv[i]));

		bool foundArg = false;
		if(iterShort != m_vecArgs.end())
		{
			// Short form matches
			setValue(*iterShort, argv[i] + iterShort->shortForm.length());
			foundArg = true;
		}
		else if(iterLong != m_vecArgs.end())
		{
			// Long form matches
			setValue(*iterLong, argv[i] + iterLong->longForm.length());
			foundArg = true;
		}


		// Check if CmdArg is known
		if(!foundArg && !ignoreUnknown)
			throw UnknownArgument(argv[i]);
	}


	if(m_hasFinalArg)
	{
		setValue(m_finalArg, argv[argc - 1]);
	}


	if(m_help)
	{
		help();
		m_stopExecution = true;
	}
	else if(m_version)
	{
		version();
		m_stopExecution = true;
	}


	// Check if required CmdArgs are all set
	if(!m_vecArgs[VERSION_ARG].isSet && !m_vecArgs[HELP_ARG].isSet)
	{
		for(std::vector<CmdArg>::iterator iterArg = m_vecArgs.begin(); \
			iterArg != m_vecArgs.end(); ++iterArg)
		{
			if(iterArg->required && !iterArg->isSet)
				throw RequiredArgumentMissing(iterArg->longForm);
		}
	}


	return;
}


void CmdApp::syntax(const std::string &message) const
{
	std::cout << m_programName << ": " << message << std::endl;

	std::cout << "Usage: " << m_programName << " [OPTION]...";
	if(m_hasFinalArg)
		std::cout << "[" << m_finalArg.longForm << "]...";
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << "Try: `" << m_programName << " --help' for more options." << std::endl;

	return;
}


bool CmdApp::stopExecution() const
{
	return m_stopExecution;
}


void CmdApp::version() const
{
	std::cout << m_programName << " " << m_programVersion << std::endl;

	return;
}


void CmdApp::help() const
{
	std::cout << m_programName << " " << m_programVersion;
	if(!m_desc.empty())
		std::cout << ", " << m_desc << ".";
	std::cout << std::endl;

	std::cout << "Usage: " << m_programName << " [OPTION]...";
	if(m_hasFinalArg)
		std::cout << "[" << m_finalArg.longForm << "]...";
	std::cout << std::endl;

	std::cout << std::endl;

	std::cout << "Options:" << std::endl;
	displayArg(m_vecArgs[VERSION_ARG]);
	displayArg(m_vecArgs[HELP_ARG]);
	std::cout << std::endl;

	for(std::vector<CmdArg>::const_iterator iterArg = m_vecArgs.begin() + 2; \
		iterArg != m_vecArgs.end(); ++iterArg)
	{
		displayArg(*iterArg);
	}

	std::cout << std::endl;
	std::cout << "Options marked with '*' are mandatory." << std::endl;

	return;
}


void CmdApp::displayArg(const CmdArg &arg) const
{
	std::cout << ((arg.required) ? '*' : ' ') << ' ';
	
	if(!arg.shortForm.empty())
		std::cout << arg.shortForm << ", ";
	else
		std::cout << "    ";

	
	std::string sLongForm = arg.longForm;
	if(!arg.param.empty())
		sLongForm += arg.param;

	std::cout << std::setiosflags(std::ios::left);
	std::cout << std::setw(FORM_WIDTH - 7) << sLongForm;
	std::cout << std::resetiosflags(std::ios::left);

	if(!arg.desc.empty())
		std::cout << " " << arg.desc << ".";   // Start pos: 31
	std::cout << std::endl;

	return;
}


void CmdApp::setValue(CmdArg &arg, const std::string &value)
{
	switch(arg.argType)
	{
	case CmdArgType::Char:
		*reinterpret_cast<char *>(arg.var) = value[0];
		break;

	case CmdArgType::Short:
		*reinterpret_cast<short *>(arg.var) = (short)atoi(value.c_str());
		break;

	case CmdArgType::Int:
		*reinterpret_cast<int *>(arg.var) = atoi(value.c_str());
		break;

	case CmdArgType::Long:
		*reinterpret_cast<__int64 *>(arg.var) = _atoi64(value.c_str());
		break;

	case CmdArgType::Float:
		*reinterpret_cast<float *>(arg.var) = (float)atof(value.c_str());
		break;

	case CmdArgType::Double:
		*reinterpret_cast<double *>(arg.var) = atof(value.c_str());
		break;

	case CmdArgType::Boolean:
		// Boolean value CmdArg is present
		*reinterpret_cast<bool *>(arg.var) = true;
		break;

	case CmdArgType::String:
		*reinterpret_cast<std::string *>(arg.var) = value;
		break;

	default:
		throw UnsupportedArgumentType();
		break;
	}

	arg.isSet = true;

	return;
}
