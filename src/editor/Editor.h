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
	APT_Material
};

struct Editor {
	Scene scene;

	Camera cam;
	Transform editorCamTrans;

	int leftBarWidth;
	int rightBarWidth;
	int topBarHeight;

	int selectedEntity;
	int selectedAxis;
	SelectionOffset selectionOffset;
	EditorGizmo gizmoType;

	float cameraCursorX;
	float cameraCursorY;

	GuiSystem gui;

	AssetPickerType pickerType;

	int GetSelectedEntity(int pixelX, int pixelY);

	Vector3 ScreenSpaceCoordsToRay(float pixelX, float pixelY);

	void HandleGizmoDrag(Entity* ent);
	void HandleGizmoClick();

	void Update();
	void Render();
	void StartUp();

	void TopPanelGui();
	void SidePanelGui();

	int IntField(int val, float x, float y, float w);
	float FloatField(float val, float x, float y, float w);
	Vector2 Vec2Field(Vector2 val, float x, float y, float w);
	Vector3 Vec3Field(Vector3 val, float x, float y, float w);

	float EditComponentGui(Component* comp, MetaStruct* meta, float x, float y, bool* outRemove);

	void DrawCurrentGizmo(const Entity* ent, Material* mat);
	void DrawSelectGizmo(const Entity* ent, Material* mat);
	void DrawPositionGizmo(const Entity* ent, Material* mat);
	void DrawRotationGizmo(const Entity* ent, Material* mat);
	void DrawScaleGizmo(const Entity* ent, Material* mat);
};

#endif
