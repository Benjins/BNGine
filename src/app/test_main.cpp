#include "app_funcs.h"
#include "../core/Scene.h"

typedef struct {
	int width;
	int height;
	int* data;
} BitmapData;

#if defined(_MSC_VER)
#pragma pack(1)
typedef struct {
#else
typedef struct __attribute((packed))__{
#endif
	short fileTag;
	int fileSize;
	short reservedA;
	short reservedB;
	int imageDataOffset;

	int headerSize;
	int imageWidth;
	int imageHeight;
	short numColorPlanes;
	short bitDepth;
	int compressionMethod;
	int imageDataSize;
	int horizontalResolution;
	int verticalResolution;
	int numPaletteColors;
	int numImportantColors;
} BitMapHeader;
#if defined(_MSC_VER)
#pragma pack()
#endif

void WriteBMPToFile(BitmapData bmp, const char* fileName) {
	FILE* bmpFile = fopen(fileName, "wb");

	if (bmpFile == NULL) {
		printf("\n\nError: could not open file '%s' for writing.\n", fileName);
		return;
	}

	int actualWidth = (((3 * bmp.width) + 3) / 4) * 4;
	int imageDataSize = actualWidth*bmp.height;

	BitMapHeader header = { 0 };
	header.bitDepth = 24;
	header.imageWidth = bmp.width;
	header.imageHeight = bmp.height;
	header.imageDataOffset = sizeof(header);
	header.imageDataSize = imageDataSize;
	header.headerSize = 40;
	header.fileTag = 0x4D42;
	header.fileSize = sizeof(header) + imageDataSize;
	header.numColorPlanes = 1;

	fwrite(&header, 1, sizeof(header), bmpFile);

	unsigned char* dataBuffer = (unsigned char*)malloc(imageDataSize);

	for (int j = 0; j < bmp.height; j++) {
		for (int i = 0; i < bmp.width; i++) {
			int bufferIndex = j * actualWidth + i * 3;
			int bmpIndex = j * bmp.width + i;
			dataBuffer[bufferIndex] = (bmp.data[bmpIndex] & 0xFF);
			dataBuffer[bufferIndex + 1] = (bmp.data[bmpIndex] & 0xFF00) >> 8;
			dataBuffer[bufferIndex + 2] = (bmp.data[bmpIndex] & 0xFF0000) >> 16;
		}
	}

	fwrite(dataBuffer, 1, imageDataSize, bmpFile);

	fclose(bmpFile);

	free(dataBuffer);
}

BitmapData LoadBMPFromFile(const char* fileName) {
	FILE* bmpFile = fopen(fileName, "rb");

	if (bmpFile == NULL) {
		printf("\n\nError: could not open file '%s'.\n", fileName);

		BitmapData empty = { 0 };
		return empty;
	}

	fseek(bmpFile, 0, SEEK_END);
	size_t fileSize = ftell(bmpFile);
	fseek(bmpFile, 0, SEEK_SET);

	unsigned char* fileBuffer = (unsigned char*)malloc(fileSize);
	fread(fileBuffer, fileSize, 1, bmpFile);
	fclose(bmpFile);

	BitMapHeader* bmpInfo = (BitMapHeader*)fileBuffer;
	int width = bmpInfo->imageWidth;
	int height = bmpInfo->imageHeight;
	int* data = (int*)malloc(4 * width*height);

	unsigned char* fileCursor = fileBuffer + bmpInfo->imageDataOffset;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int pixel = fileCursor[0] | (fileCursor[1] << 8) | (fileCursor[2] << 16);
			fileCursor += 3;
			data[j*width + i] = pixel;
		}
	}

	free(fileBuffer);
	BitmapData bmpData = { width, height, data };
	return bmpData;
}

bool CompareFrameBufferAndWriteFile(BitmapData fb, const char* fileName) {
	char refImageFile[256] = {};
	snprintf(refImageFile, sizeof(refImageFile), "tests/ref_img/%s", fileName);

	BitmapData refBmp = LoadBMPFromFile(refImageFile);

	if (refBmp.width != fb.width || refBmp.height != fb.height) {
		printf("\nError: Images '%s' and '%s' differ in dimensions.  Failure.\n", fileName, refImageFile);
		WriteBMPToFile(fb, fileName);
		free(refBmp.data);
		return false;
	}

	const int tolerance = 5;
	const int maxDiffPixelCount = 10;
	int diffPixelCount = 0;

	for (int i = 0; i < refBmp.width * refBmp.height; i++) {
		int p1 = (refBmp.data[i] & 0x00FFFFFF);
		int p2 = (fb.data[i] & 0x00FFFFFF);
		int p1_c[3] = { (p1 & 0xFF), (p1 & 0xFF00) >> 8, (p1 & 0x00FF0000) >> 16 };
		int p2_c[3] = { (p2 & 0xFF), (p2 & 0xFF00) >> 8, (p2 & 0x00FF0000) >> 16 };

		int diff = 0;
		for (int k = 0; k < 3; k++) {
			diff += BNS_ABS(p1_c[k] - p2_c[k]);
		}

		if (diff > tolerance) {
			diffPixelCount++;
		}

		if (diffPixelCount > maxDiffPixelCount) {
			printf("\nError: Images '%s' and '%s' differ at more than %d pixels. Failure.\n", fileName, refImageFile, maxDiffPixelCount);
			WriteBMPToFile(fb, fileName);
			free(refBmp.data);
			return false;
		}

	}

	printf("Images '%s' and '%s' match.\n", fileName, refImageFile);
	free(refBmp.data);
	return true;
}

void AppPostInit(int argc, char** argv) {
	Scene scn;

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);
	glLoadIdentity();

	scn.StartUp();

	scn.LockFrameRate(0.02f);

	scn.input.SetCursorPos(0, 0);
}

bool doCompare = false;

bool AppUpdate(int argc, char** argv) {
	for (int i = 0; i < 5; i++) {
		scn.Update();
		scn.Render();
	}

	BitmapData frameData = {};
	frameData.width = width;
	frameData.height = height;
	frameData.data = (int*)imgBuffer;

	bool success = true;
	if (doCompare) {
		success = CompareFrameBufferAndWriteFile(frameData, "frame_04.bmp");
	}

	if (!success) {
		OSMesaDestroyContext(ctx);

		free(imgBuffer);
		return -1;
	}

	scn.LoadLevel("Test_Level.lvl");

	for (int i = 0; i < 5; i++) {
		scn.Update();
		scn.Render();
	}

	if (doCompare) {
		success = CompareFrameBufferAndWriteFile(frameData, "Test_Level.bmp");
	}

	return success;
}

void AppShutdown(int argc, char** argv) {
	GlobalScene->ShutDown();
}

void AppMouseMove(int x, int y) {
	GlobalScene->input.SetCursorPos(x, y);
}

void AppMouseUp(int button) {
	GlobalScene->input.MouseButtonReleased((MouseButton)button);
}

void AppMouseDown(int button) {
	GlobalScene->input.MouseButtonPressed((MouseButton)button);
}

void AppKeyUp(unsigned char key) {
	GlobalScene->input.KeyReleased(key);
}

void AppKeyDown(unsigned char key) {
	GlobalScene->input.KeyPressed(key);
}

void AppPreInit(int argc, char** argv) {
	if (argc > 1 && StrEqual(argv[1], "--compare")) {
		doCompare = true;
	}
}

void AppSetWindowSize(int w, int h) {
	GlobalScene->cam.widthPixels = w;
	GlobalScene->cam.heightPixels = h;
}

