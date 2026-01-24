#pragma once

#include <cstdint>
#include <cstddef>

// All code here was build based on a lot of hard work done my the reverse engineering community of the RAGE engine.
// Some of this was taken from some 'leaked' src and I probably shouldnt say that (Rockstar pls dont come after me). 
// Anyways a lot of the information and code for RAGE.h and RAGE.inl was from multiple different reverse engineering projects.
// My sources will be in docs

namespace rage
{
    class datBase;
    class pgBase;
    class grcDevice;
    class grmShaderGroup;
    class fiDevice;
    class fiPackfile;
    class phBound;
    class scrThread;
    class scrProgram;
    class audEngine;
    template<typename T> class atArray;
    template<typename T> class atHashMap;
}

namespace GTA4
{
    class CEntity;
    class CPhysical;
    class CDynamicEntity;
    class CPed;
    class CPlayerPed;
    class CVehicle;
    class CAutomobile;
    class CObject;
    class CBuilding;
    class CCamera;
    class CWorld;
    class CGame;
    class CStreaming;
    class CTimer;
}

namespace rage
{
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using s8  = int8_t;
    using s16 = int16_t;
    using s32 = int32_t;
    using s64 = int64_t;
    using f32 = float;
    using f64 = double;

    struct Vector2
    {
        f32 x, y;
    };

    struct Vector3
    {
        f32 x, y, z;
        
        Vector3() : x(0), y(0), z(0) {}
        Vector3(f32 x_, f32 y_, f32 z_) : x(x_), y(y_), z(z_) {}
    };

    struct Vector4
    {
        f32 x, y, z, w;
        
        Vector4() : x(0), y(0), z(0), w(0) {}
        Vector4(f32 x_, f32 y_, f32 z_, f32 w_) : x(x_), y(y_), z(z_), w(w_) {}
    };

    struct Matrix34
    {
        Vector4 right;
        Vector4 forward;
        Vector4 up;
        Vector4 pos;
    };

    struct Matrix44
    {
        f32 m[4][4];
    };

    template<typename T>
    class atArray
    {
    public:
        T* m_data;
        u16 m_size;
        u16 m_capacity;

        atArray() : m_data(nullptr), m_size(0), m_capacity(0) {}
        
        u16 GetSize() const { return m_size; }
        u16 GetCapacity() const { return m_capacity; }
        T* GetData() { return m_data; }
        const T* GetData() const { return m_data; }
        
        T& operator[](u16 index) { return m_data[index]; }
        const T& operator[](u16 index) const { return m_data[index]; }
    };

    template<typename T>
    class atHashMap
    {
    public:
        struct Entry
        {
            u32 hash;
            T value;
            Entry* next;
        };

        Entry** m_buckets;
        u32 m_bucketCount;
        u32 m_size;
    };

    class datBase
    {
    public:
        virtual ~datBase() = default;
    };

    class pgBase : public datBase
    {
    public:
        u32 m_blockMap;
        
        virtual ~pgBase() = default;
    };

    class fiDevice
    {
    public:
        virtual ~fiDevice() = default;
        virtual u32 Open(const char* path, bool readOnly) = 0;
        virtual u32 OpenBulk(const char* path, u64* size) = 0;
        virtual u32 Create(const char* path) = 0;
        virtual u32 Read(u32 handle, void* buffer, u32 size) = 0;
        virtual u32 Write(u32 handle, const void* buffer, u32 size) = 0;
        virtual u32 Seek(u32 handle, s32 offset, u32 origin) = 0;
        virtual u64 SeekLong(u32 handle, s64 offset, u32 origin) = 0;
        virtual void Close(u32 handle) = 0;
        virtual u64 GetFileSize(const char* path) = 0;
    };

    class fiPackfile : public fiDevice
    {
    public:
        char m_name[256];
        u32 m_handle;
        u64 m_baseOffset;
        
        virtual ~fiPackfile() = default;
    };

    class grcDevice
    {
    public:
        static grcDevice* sm_Instance;
        
        u32 m_width;
        u32 m_height;
        void* m_d3dDevice;
        
        virtual ~grcDevice() = default;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Present() = 0;
    };

    class grcTexture : public pgBase
    {
    public:
        u16 m_width;
        u16 m_height;
        u8 m_format;
        u8 m_mipLevels;
        void* m_textureData;
    };

    class grmShader : public pgBase
    {
    public:
        u32 m_hash;
        atArray<u32> m_parameters;
    };

    class grmShaderGroup : public pgBase
    {
    public:
        atArray<grmShader*> m_shaders;
        atArray<grcTexture*> m_textures;
    };

    class phBound : public pgBase
    {
    public:
        u8 m_type;
        u8 m_flags;
        u16 m_partIndex;
        f32 m_radius;
        Vector3 m_boundingBoxMin;
        Vector3 m_boundingBoxMax;
        Vector3 m_boundingSphereCenter;
        f32 m_margin;
    };

    class phBoundComposite : public phBound
    {
    public:
        atArray<phBound*> m_bounds;
        atArray<Matrix34> m_matrices;
    };

    class scrProgram : public pgBase
    {
    public:
        u8* m_codeBlock;
        u32 m_codeSize;
        u32* m_stringTable;
        u32 m_stringCount;
        u32* m_nativeTable;
        u32 m_nativeCount;
        u32 m_localVarCount;
        u32 m_globalVarCount;
    };

    class scrThread
    {
    public:
        u32 m_threadId;
        u32 m_scriptHash;
        u32 m_state;
        u32 m_instructionPointer;
        u32 m_framePointer;
        u32 m_stackPointer;
        f32 m_waitTime;
        u8* m_stack;
        u32 m_stackSize;
        scrProgram* m_program;
        
        virtual void Reset(u32 scriptHash, void* args, u32 argCount) = 0;
        virtual u32 Run(u32 instructionCount) = 0;
        virtual void Kill() = 0;
    };

    class audSound
    {
    public:
        u32 m_id;
        u32 m_hash;
        u8 m_state;
        f32 m_volume;
        f32 m_pitch;
        Vector3 m_position;
    };

    class audEngine
    {
    public:
        static audEngine* sm_Instance;
        
        virtual void Update(f32 deltaTime) = 0;
        virtual audSound* PlaySound(u32 hash) = 0;
        virtual audSound* Play3DSound(u32 hash, const Vector3& position) = 0;
        virtual void StopSound(audSound* sound) = 0;
        virtual void StopAllSounds() = 0;
    };

}

namespace GTA4
{
    using namespace rage;

    class CEntity : public pgBase
    {
    public:
        Matrix34 m_matrix;
        u32 m_flags;
        u8 m_type;
        u8 m_status;
        u16 m_randomSeed;
        void* m_modelInfo;
        void* m_drawable;
        
        virtual ~CEntity() = default;
        virtual void SetMatrix(const Matrix34& matrix) { m_matrix = matrix; }
        virtual const Matrix34& GetMatrix() const { return m_matrix; }
        virtual Vector3 GetPosition() const { return Vector3(m_matrix.pos.x, m_matrix.pos.y, m_matrix.pos.z); }
        virtual void SetPosition(const Vector3& pos) { m_matrix.pos.x = pos.x; m_matrix.pos.y = pos.y; m_matrix.pos.z = pos.z; }
    };

    class CPhysical : public CEntity
    {
    public:
        void* m_physics;
        Vector3 m_velocity;
        Vector3 m_angularVelocity;
        f32 m_mass;
        u8 m_physicsFlags;
        
        virtual void ApplyForce(const Vector3& force, const Vector3& offset) = 0;
        virtual void ApplyImpulse(const Vector3& impulse, const Vector3& offset) = 0;
    };

    class CDynamicEntity : public CPhysical
    {
    public:
        u32 m_lastFrameUpdate;
    };

    class CPed : public CPhysical
    {
    public:
        u32 m_pedType;
        u32 m_pedState;
        f32 m_health;
        f32 m_maxHealth;
        f32 m_armour;
        void* m_weapons;
        u8 m_currentWeapon;
        void* m_pedIntelligence;
        void* m_playerInfo;
        void* m_animator;
        
        virtual bool IsPlayer() const { return m_playerInfo != nullptr; }
        virtual bool IsAlive() const { return m_health > 0; }
        virtual void SetHealth(f32 health) { m_health = health; }
        virtual void Kill() { m_health = 0; }
    };

    class CPlayerPed : public CPed
    {
    public:
        u32 m_playerId;
        f32 m_stamina;
        f32 m_wantedLevel;
        u32 m_money;
        
        virtual void AddMoney(s32 amount) { m_money += amount; }
        virtual void SetWantedLevel(u32 level) { m_wantedLevel = (f32)level; }
    };

    class CVehicle : public CPhysical
    {
    public:
        u8 m_vehicleType;
        u8 m_numSeats;
        u8 m_numPassengers;
        f32 m_health;
        f32 m_engineHealth;
        f32 m_petrolTankHealth;
        f32 m_dirt;
        u32 m_primaryColor;
        u32 m_secondaryColor;
        CPed* m_driver;
        CPed* m_passengers[8];
        void* m_handling;
        
        virtual bool IsDestroyed() const { return m_health <= 0; }
        virtual u8 GetNumPassengers() const { return m_numPassengers; }
        virtual CPed* GetDriver() const { return m_driver; }
    };

    class CAutomobile : public CVehicle
    {
    public:
        f32 m_wheelRotation[4];
        f32 m_wheelSpeed[4];
        f32 m_steerAngle;
        f32 m_gasPedal;
        f32 m_brakePedal;
        u8 m_currentGear;
        f32 m_rpm;
    };

    class CBike : public CVehicle
    {
    public:
        f32 m_leanAngle;
    };

    class CBoat : public CVehicle
    {
    public:
        f32 m_throttle;
        f32 m_rudderAngle;
    };

    class CHeli : public CVehicle
    {
    public:
        f32 m_rotorSpeed;
        f32 m_rotorRotation;
    };

    class CObject : public CPhysical
    {
    public:
        u32 m_objectFlags;
        f32 m_objectHealth;
    };

    class CBuilding : public CEntity
    {
    public:
    };

    class CPickup : public CObject
    {
    public:
        u32 m_pickupType;
        u32 m_amount;
        f32 m_regenerateTime;
    };

    class CCamera
    {
    public:
        static CCamera* sm_Instance;
        
        Matrix34 m_matrix;
        f32 m_fov;
        f32 m_nearClip;
        f32 m_farClip;
        f32 m_aspectRatio;
        u8 m_cameraMode;
        CEntity* m_targetEntity;
        
        virtual void Update(f32 deltaTime) = 0;
        virtual void SetPosition(const Vector3& pos) = 0;
        virtual void LookAt(const Vector3& target) = 0;
    };

    class CWorld
    {
    public:
        static CWorld* sm_Instance;
        
        atArray<CEntity*> m_entities;
        atArray<CPed*> m_peds;
        atArray<CVehicle*> m_vehicles;
        atArray<CObject*> m_objects;
        CPlayerPed* m_playerPed;
        
        virtual void Add(CEntity* entity) = 0;
        virtual void Remove(CEntity* entity) = 0;
        virtual void Process(f32 deltaTime) = 0;
    };

    class CStreaming
    {
    public:
        static CStreaming* sm_Instance;
        
        u32 m_memoryUsed;
        u32 m_memoryLimit;
        
        virtual void RequestModel(u32 modelId) = 0;
        virtual bool HasModelLoaded(u32 modelId) = 0;
        virtual void LoadAllRequestedModels() = 0;
        virtual void ReleaseModel(u32 modelId) = 0;
    };

    class CTimer
    {
    public:
        static u32 m_frameCounter;
        static f32 m_timeStep;
        static f32 m_timeStepNonClipped;
        static u32 m_gameTimer;
        static u32 m_systemTimer;
        static f32 m_timeScale;
        static bool m_paused;
        
        static void Update();
        static f32 GetTimeStep() { return m_timeStep; }
        static u32 GetFrameCounter() { return m_frameCounter; }
    };

    class CGame
    {
    public:
        static CGame* sm_Instance;
        
        u8 m_gameState;
        bool m_isInitialized;
        bool m_isPaused;
        
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void Process() = 0;
        virtual void Render() = 0;
    };

    template<typename T>
    class CPool
    {
    public:
        T* m_objects;
        u8* m_flags;
        u32 m_size;
        u32 m_firstFree;
        
        T* GetAt(u32 index)
        {
            if (index < m_size && (m_flags[index] & 0x80) == 0)
                return &m_objects[index];
            return nullptr;
        }
        
        u32 GetSize() const { return m_size; }
    };

    extern CPool<CPed>* g_pedPool;
    extern CPool<CVehicle>* g_vehiclePool;
    extern CPool<CObject>* g_objectPool;
    extern CPool<CBuilding>* g_buildingPool;

}

namespace rage
{
    inline grcDevice* grcDevice::sm_Instance = nullptr;
    inline audEngine* audEngine::sm_Instance = nullptr;
}

namespace GTA4
{
    inline CCamera* CCamera::sm_Instance = nullptr;
    inline CWorld* CWorld::sm_Instance = nullptr;
    inline CStreaming* CStreaming::sm_Instance = nullptr;
    inline CGame* CGame::sm_Instance = nullptr;
    
    inline CPool<CPed>* g_pedPool = nullptr;
    inline CPool<CVehicle>* g_vehiclePool = nullptr;
    inline CPool<CObject>* g_objectPool = nullptr;
    inline CPool<CBuilding>* g_buildingPool = nullptr;
    
    inline u32 CTimer::m_frameCounter = 0;
    inline f32 CTimer::m_timeStep = 0.0f;
    inline f32 CTimer::m_timeStepNonClipped = 0.0f;
    inline u32 CTimer::m_gameTimer = 0;
    inline u32 CTimer::m_systemTimer = 0;
    inline f32 CTimer::m_timeScale = 1.0f;
    inline bool CTimer::m_paused = false;
}
