#include "../ext/3dbasics/Basis.cpp"
#include "../ext/3dbasics/Box.cpp"
#include "../ext/3dbasics/Mat4.cpp"
#include "../ext/3dbasics/Matrix.cpp"
#include "../ext/3dbasics/Plane.cpp"
#include "../ext/3dbasics/Quaternion.cpp"
#include "../ext/3dbasics/Ray.cpp"
#include "../ext/3dbasics/Sphere.cpp"
#include "../ext/3dbasics/Triangle.cpp"
#include "../ext/3dbasics/Vector2.cpp"
#include "../ext/3dbasics/Vector3.cpp"
#include "../ext/3dbasics/Vector4.cpp"
#include "../ext/CppUtils/assert.cpp"
#include "../ext/CppUtils/bnvm.cpp"
#include "../ext/CppUtils/bnvparser.cpp"
#include "../ext/CppUtils/combinedtest.cpp"
#include "../ext/CppUtils/filesys.cpp"
#include "../ext/CppUtils/hash.cpp"
#include "../ext/CppUtils/idbase.cpp"
#include "../ext/CppUtils/lexer.cpp"
#include "../ext/CppUtils/macros_test.cpp"
#include "../ext/CppUtils/memstream.cpp"
#include "../ext/CppUtils/ref.cpp"
#include "../ext/CppUtils/stringmap.cpp"
#include "../ext/CppUtils/strings.cpp"
#include "../ext/CppUtils/vector.cpp"
#include "../ext/CppUtils/xml.cpp"
#include "../src/assets/AssetFile.cpp"
#include "../src/core/Camera.cpp"
#include "../src/core/Component.cpp"
#include "../src/core/Entity.cpp"
#include "../src/core/Input.cpp"
#include "../src/core/Scene.cpp"
#include "../src/core/Transform.cpp"
#include "../src/game/Player.cpp"
#include "../src/gfx/BitmapFont.cpp"
#include "../src/gfx/DrawCall.cpp"
#include "../src/gfx/GLExtInit.cpp"
#include "../src/gfx/Material.cpp"
#include "../src/gfx/Mesh.cpp"
#include "../src/gfx/Program.cpp"
#include "../src/gfx/ResourceManager.cpp"
#include "../src/gfx/Shader.cpp"
#include "../src/gfx/Texture.cpp"
#include "../src/gui/GuiSystem.cpp"
#include "../src/metagen/MetaParse.cpp"
#include "../src/physics/BoxCollider.cpp"
#include "../src/physics/Collision.cpp"
#include "../src/physics/PhysicsSystem.cpp"
#include "../src/util/ComponentMetaHelpers.cpp"
#include "../src/util/Serialization.cpp"
