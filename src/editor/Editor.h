#ifndef EDITOR_H
#define EDITOR_H

#pragma once

#include "../core/Scene.h"

enum EditorGizmo {
	EG_Select,
	EG_Position,
	EG_Rotation,
	EG_Scale
};

union SelectionOffset {
	Vector3 position;
	float rotation;

	SelectionOffset() {
		position = Vector3(0, 0, 0);
		rotation = 0;
	}
};

struct MetaStruct;

enum AssetPickerType {
	APT_None,
	APT_ComponentType,
	APT_Mesh,
	APT_Material,
	APT_Prefab,
	APT_EditorView,
	APT_Level
};

enum CopyPasteType {
	CPT_Entity,
	CPT_Count
};

struct CopyPasteData {
	CopyPasteType type;
	union {
		int entityId;
	};
};

enum EditorView {
	EV_None = -1,
	EV_Scene,
	EV_Prefab,
	EV_Count
};

struct Editor {
	Scene scene;

	Camera cam;
	Transform editorCamTrans;

	EditorView currentView;

	int leftBarWidth;
	int rightBarWidth;
	int topBarHeight;

	// Scene view -------
	int selectedEntity;
	int selectedAxis;
	SelectionOffset selectionOffset;
	EditorGizmo gizmoType;

	float cameraCursorX;
	float cameraCursorY;
	// ------------

	// Prefab view ----
	int selectedPrefab;
	//------------

	GuiSystem gui;

	AssetPickerType pickerType;

	CopyPasteData copyPasteData;

	Editor() {
		currentView = EV_Scene;
	}

	void SwitchView(EditorView newView);

	int GetSelectedEntity(int pixelX, int pixelY);

	Vector3 ScreenSpaceCoordsToRay(float pixelX, float pixelY);

	void HandleGizmoDrag(Entity* ent);
	void HandleGizmoClick();

	void Update();
	void Render();
	void StartUp();

	void RenderPrefab();

	void TopPanelGui();
	void SidePanelGui();

	int IntField(int val, float x, float y, float w);
	float FloatField(float val, float x, float y, float w);
	bool BoolField(bool val, float x, float y, float w);
	Vector2 Vec2Field(Vector2 val, float x, float y, float w);
	Vector3 Vec3Field(Vector3 val, float x, float y, float w);

	float EditComponentGui(Component* comp, MetaStruct* meta, float x, float y, bool* outRemove);

	void SaveScene();
	void SavePrefab();

	void DrawCurrentGizmo(const Entity* ent, Material* mat);
	void DrawSelectGizmo(const Entity* ent, Material* mat);
	void DrawPositionGizmo(const Entity* ent, Material* mat);
	void DrawRotationGizmo(const Entity* ent, Material* mat);
	void DrawScaleGizmo(const Entity* ent, Material* mat);
};

/*[Action]*/
void EditorShiftButton(Editor* ed, uint32 buttonId);

/*[Action]*/
void EditorResetButton(Editor* ed, uint32 buttonId);

/*[Action]*/
void EditorAddStringPicker(Editor* ed, int enumIndex, int buttonId, Vector2 pos, Vector2 size);

/*[Action]*/
void EditorPrintEnum(Editor* ed, int pickerId, int buttonId);

#endif
