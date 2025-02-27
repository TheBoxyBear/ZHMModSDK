#pragma once

#include <random>
#include <unordered_map>
#include <map>
#include <shared_mutex>

#include "IPluginInterface.h"
#include "Glacier/ZEntity.h"
#include "Glacier/ZInput.h"

#include "Components/Qne.h"

#include "ImGuizmo.h"

struct EntityTreeNode {
	std::string Name;
	uint64_t EntityId;
	ZEntityRef Entity;
	std::multimap<std::string, std::shared_ptr<EntityTreeNode>> Children;

	EntityTreeNode(const std::string& p_Name, uint64_t p_EntityId, ZEntityRef p_Ref)
		: Name(p_Name), EntityId(p_EntityId), Entity(p_Ref) {}
};

class Editor : public IPluginInterface
{
public:
    Editor();
    ~Editor() override;

    void Init() override;
    void OnDrawMenu() override;
    void OnDrawUI(bool p_HasFocus) override;
    void OnDraw3D(IRenderer* p_Renderer) override;
    void OnEngineInitialized() override;

private:
    void SpawnCameras();
    void CopyToClipboard(const std::string& p_String) const;
    void OnMouseDown(SVector2 p_Pos, bool p_FirstClick);

    void DrawEntityManipulator(bool p_HasFocus);
    void DrawEntityAABB(IRenderer* p_Renderer);

    void DrawEntityProperties();

    void RenderEntity(std::shared_ptr<EntityTreeNode> p_Node);
    void DrawEntityTree();
    bool SearchForEntityById(ZTemplateEntityBlueprintFactory* p_BrickFactory, ZEntityRef p_BrickEntity, uint64_t p_EntityId);
    bool SearchForEntityByType(ZTemplateEntityBlueprintFactory* p_BrickFactory, ZEntityRef p_BrickEntity, const std::string& p_TypeName);
    bool SearchForEntityByName(ZTemplateEntityBlueprintFactory* p_BrickFactory, ZEntityRef p_BrickEntity, const std::string& p_EntityName);
	void UpdateEntities();

    void OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent);
    void CheckQneConnection(float p_DeltaTime);
    void ReceiveQneMessages();
    void SendQneMessage(const Qne::SdkToQnMessage& p_Message);

    void DrawPinTracer();

    static bool ImGuiCopyWidget(const std::string& p_Id);

private:
    DECLARE_PLUGIN_DETOUR(Editor, void, OnLoadScene, ZEntitySceneContext*, ZSceneData&);
    DECLARE_PLUGIN_DETOUR(Editor, void, OnClearScene, ZEntitySceneContext* th, bool fullyClear);
    DECLARE_PLUGIN_DETOUR(Editor, ZTemplateEntityBlueprintFactory*, ZTemplateEntityBlueprintFactory_ctor, ZTemplateEntityBlueprintFactory* th, STemplateEntityBlueprint* pTemplateEntityBlueprint, ZResourcePending& ResourcePending);
    DECLARE_PLUGIN_DETOUR(Editor, bool, OnInputPin, ZEntityRef entity, uint32_t pinId, const ZObjectRef& data);
    DECLARE_PLUGIN_DETOUR(Editor, bool, OnOutputPin, ZEntityRef entity, uint32_t pinId, const ZObjectRef& data);

private:
    ZEntityRef m_Camera;
    ZEntityRef m_CameraRT;

    bool m_HoldingMouse = false;
    bool m_UseSnap = false;
    float m_SnapValue[3] = { 1.0f, 1.0f, 1.0f };

    float4 m_From;
    float4 m_To;
    float4 m_Hit;
    float4 m_Normal;

    size_t m_SelectedBrickIndex = 0;
    ZEntityRef m_SelectedEntity;
    bool m_ShouldScrollToEntity = false;

    ImGuizmo::OPERATION m_GizmoMode = ImGuizmo::OPERATION::TRANSLATE;
    ImGuizmo::MODE m_GizmoSpace = ImGuizmo::MODE::WORLD;

    ZInputAction m_RaycastAction = "SkipLoadingAction"; // space
    ZInputAction m_DeleteAction = "TemporaryCamSpeedMultiplier0"; // shift

    struct PinFireInfo
    {
        std::chrono::time_point<std::chrono::system_clock> m_FireTime;
    };

    std::unordered_map<uint32_t, PinFireInfo> m_FiredInputPins = {};
    std::unordered_map<uint32_t, PinFireInfo> m_FiredOutputPins = {};

    SOCKET m_QneSocket = INVALID_SOCKET;
    bool m_ConnectedToQne = false;
    float m_QneConnectionTimer = 999.f; // Set to a high number so we connect on startup.
    sockaddr_in m_QneAddress = {};

	std::shared_mutex m_CachedEntityTreeMutex;
	std::shared_ptr<EntityTreeNode> m_CachedEntityTree;
};

DECLARE_ZHM_PLUGIN(Editor)
