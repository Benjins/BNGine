#include "../../ext/CppUtils/filesys.cpp"
#include "../../ext/CppUtils/strings.cpp"

enum Platform {
	P_X11,
	P_Win32,
	P_Console,
	P_OSMesa,
	P_Count
};

enum Application {
	A_Runtime,
	A_MetaGen,
	A_Editor,
	A_Test,
	A_Server,
	A_Count
};

const char* platformNames[P_Count] = {
	"x11",
	"win32",
	"console",
	"osmesa"
};

const char* appNames[A_Count] = {
	"runtime",
	"metagen",
	"editor",
	"test",
	"server"
};

// XXX: HACK: Ehhh....yeah.
// This is to avoid having to refactor the build pipeline...
// Not the best solution
bool forceConsoleApp[A_Count] = {
	false,
	true,
	false,
	false,
	true
};

int main(int argc, char** argv) {
	File srcDir;
	srcDir.Load("src");

	File extDir;
	extDir.Load("ext");

	Vector<const char*> exceptions;
	exceptions.PushBack("app");
	exceptions.PushBack("platform");
	exceptions.PushBack("unity");

	Vector<const char*> includeFirst;
	includeFirst.PushBack("GLExtInit.cpp");

	if (argc != 3) {
		printf("Need an application and a platform in args!\n");
		exit(-1);
	}

	Platform platform = P_Count;
	for (int i = 0; i < P_Count; i++) {
		if (StrEqual(platformNames[i], argv[1])) {
			platform = (Platform)i;
			break;
		}
	}

	Application app = A_Count;
	for (int i = 0; i < A_Count; i++) {
		if (StrEqual(appNames[i], argv[2])) {
			app = (Application)i;
			break;
		}
	}

	if (app == A_Count) {
		printf("Could not find app '%s'\n.", argv[2]);
		exit(-1);
	}

	if (forceConsoleApp[(int)app]) {
		platform = P_Console;
	}
	else {
		if (platform == P_Count) {
			printf("Could not find platform '%s'\n.", argv[1]);
			exit(-1);
		}
	}

	if (app != A_Editor) {
		exceptions.PushBack("editor");
	}

	Vector<File*> srcFiles;
	{
		Vector<File*> extSrcFiles;
		extDir.FindFilesWithExt("cpp", &extSrcFiles);
		for (int j = 0; j < extSrcFiles.count; j++) {
			srcFiles.PushBack(extSrcFiles.Get(j));
		}
	}

	if (app == A_MetaGen) {
		Vector<File*> srcSubFiles;
		srcDir.Find("metagen")->FindFilesWithExt("cpp", &srcSubFiles);

		for (int j = 0; j < srcSubFiles.count; j++) {
			srcFiles.PushBack(srcSubFiles.Get(j));
		}
	}
	else {
		for (int i = 0; i < srcDir.childCount; i++) {
			bool allowed = true;
			for (int j = 0; j < exceptions.count; j++) {
				if (!strcmp(srcDir.children[i].fileName, exceptions.Get(j))) {
					allowed = false;
					break;
				}
			}

			if (allowed) {
				Vector<File*> srcSubFiles;
				srcDir.children[i].FindFilesWithExt("cpp", &srcSubFiles);

				for (int j = 0; j < srcSubFiles.count; j++) {
					srcFiles.PushBack(srcSubFiles.Get(j));
				}
			}
		}
	}

	FILE* genFile = fopen("gen/UnityBuild.cpp", "wb");
	const char* fileToTop = "../";
	
	// HACK: The Windows API doesn't like incuding Winsock2.h after Windows.h, so this is a workaround
	fprintf(genFile, "#define _WINSOCKAPI_\n");

	if (app == A_Editor) {
		fprintf(genFile, "#define BNS_EDITOR\n");
	}

	for (int i = 0; i < includeFirst.count; i++) {
		for (int j = 0; j < srcFiles.count; j++) {
			if (!strcmp(srcFiles.Get(j)->fileName, includeFirst.Get(i))) {
				fprintf(genFile, "#include \"%s%s\"\n", fileToTop, srcFiles.Get(j)->fullName);
				srcFiles.Remove(j);
				break;
			}
		}
	}
	
	for (int i = 0; i < srcFiles.count; i++) {
		fprintf(genFile, "#include \"%s%s\"\n", fileToTop, srcFiles.Get(i)->fullName);
	}

	const char* appFiles[A_Count] = {
		"src/app/runtime_main.cpp",
		"src/app/metagen_main.cpp",
		"src/app/editor_main.cpp",
		"src/app/test_main.cpp",
		"src/app/matchmaking_server.cpp"
	};

	const char* platformFiles[P_Count] = {
		"src/platform/x11_entry_main.cpp",
		"src/platform/win32_entry_main.cpp",
		"src/platform/console_entry_main.cpp",
		"src/platform/osmesa_entry_main.cpp"
	};

	fprintf(genFile, "#include \"%s%s\"\n", fileToTop, appFiles[(int)app]);
	fprintf(genFile, "#include \"%s%s\"\n", fileToTop, platformFiles[(int)platform]);

	// For everything except metagen, we want to include all of our generated files in the unity build
	// except for itself
	if (app != A_MetaGen) {
		File genDir;
		genDir.Load("gen");

		Vector<File*> genFiles;
		genDir.FindFilesWithExt("cpp", &genFiles);
		for (int j = 0; j < genFiles.count; j++) {
			if (!StrEqual(genFiles.Get(j)->fileName, "UnityBuild.cpp")) {
				fprintf(genFile, "#include \"%s\"\n", genFiles.Get(j)->fileName);
			}
		}
	}

	fclose(genFile);

	return 0;
}
