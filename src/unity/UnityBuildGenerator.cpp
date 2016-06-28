#include "../../ext/CppUtils/filesys.cpp"

int main(int argc, char** argv) {

	File srcDir;
	srcDir.Load("src");

	File extDir;
	extDir.Load("ext");

	Vector<const char*> exceptions;
	exceptions.PushBack("app");
	exceptions.PushBack("unity");

	Vector<const char*> includeFirst;
	includeFirst.PushBack("GLExtInit.cpp");

	bool isEditor = false;
	bool isMetaGen = false;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[1], "-e")) {
			isEditor = true;
		}
		else if (!strcmp(argv[1], "-mg")) {
			isMetaGen = true;
		}
	}

	if (!isEditor) {
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

	if (isMetaGen) {
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

	fclose(genFile);

	return 0;
}
