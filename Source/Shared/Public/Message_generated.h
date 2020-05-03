// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MESSAGE_AM_FB_H_
#define FLATBUFFERS_GENERATED_MESSAGE_AM_FB_H_

#include "flatbuffers/flatbuffers.h"

namespace AM {
namespace fb {

struct ConnectionRequest;
struct ConnectionRequestBuilder;

struct ConnectionResponse;
struct ConnectionResponseBuilder;

struct InputComponent;
struct InputComponentBuilder;

struct PositionComponent;
struct PositionComponentBuilder;

struct MovementComponent;
struct MovementComponentBuilder;

struct Rect;

struct SpriteComponent;

struct Entity;
struct EntityBuilder;

struct EntityUpdate;
struct EntityUpdateBuilder;

struct Message;
struct MessageBuilder;

enum class InputState : int8_t {
  Invalid = 0,
  Pressed = 1,
  Released = 2,
  MIN = Invalid,
  MAX = Released
};

inline const InputState (&EnumValuesInputState())[3] {
  static const InputState values[] = {
    InputState::Invalid,
    InputState::Pressed,
    InputState::Released
  };
  return values;
}

inline const char * const *EnumNamesInputState() {
  static const char * const names[4] = {
    "Invalid",
    "Pressed",
    "Released",
    nullptr
  };
  return names;
}

inline const char *EnumNameInputState(InputState e) {
  if (flatbuffers::IsOutRange(e, InputState::Invalid, InputState::Released)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesInputState()[index];
}

enum class MessageContent : uint8_t {
  NONE = 0,
  ConnectionRequest = 1,
  ConnectionResponse = 2,
  EntityUpdate = 3,
  MIN = NONE,
  MAX = EntityUpdate
};

inline const MessageContent (&EnumValuesMessageContent())[4] {
  static const MessageContent values[] = {
    MessageContent::NONE,
    MessageContent::ConnectionRequest,
    MessageContent::ConnectionResponse,
    MessageContent::EntityUpdate
  };
  return values;
}

inline const char * const *EnumNamesMessageContent() {
  static const char * const names[5] = {
    "NONE",
    "ConnectionRequest",
    "ConnectionResponse",
    "EntityUpdate",
    nullptr
  };
  return names;
}

inline const char *EnumNameMessageContent(MessageContent e) {
  if (flatbuffers::IsOutRange(e, MessageContent::NONE, MessageContent::EntityUpdate)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMessageContent()[index];
}

template<typename T> struct MessageContentTraits {
  static const MessageContent enum_value = MessageContent::NONE;
};

template<> struct MessageContentTraits<AM::fb::ConnectionRequest> {
  static const MessageContent enum_value = MessageContent::ConnectionRequest;
};

template<> struct MessageContentTraits<AM::fb::ConnectionResponse> {
  static const MessageContent enum_value = MessageContent::ConnectionResponse;
};

template<> struct MessageContentTraits<AM::fb::EntityUpdate> {
  static const MessageContent enum_value = MessageContent::EntityUpdate;
};

bool VerifyMessageContent(flatbuffers::Verifier &verifier, const void *obj, MessageContent type);
bool VerifyMessageContentVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Rect FLATBUFFERS_FINAL_CLASS {
 private:
  int32_t x_;
  int32_t y_;
  int32_t w_;
  int32_t h_;

 public:
  Rect() {
    memset(static_cast<void *>(this), 0, sizeof(Rect));
  }
  Rect(int32_t _x, int32_t _y, int32_t _w, int32_t _h)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)),
        w_(flatbuffers::EndianScalar(_w)),
        h_(flatbuffers::EndianScalar(_h)) {
  }
  int32_t x() const {
    return flatbuffers::EndianScalar(x_);
  }
  int32_t y() const {
    return flatbuffers::EndianScalar(y_);
  }
  int32_t w() const {
    return flatbuffers::EndianScalar(w_);
  }
  int32_t h() const {
    return flatbuffers::EndianScalar(h_);
  }
};
FLATBUFFERS_STRUCT_END(Rect, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) SpriteComponent FLATBUFFERS_FINAL_CLASS {
 private:
  int16_t textureID_;
  int16_t padding0__;
  AM::fb::Rect posInTexture_;
  int32_t width_;
  int32_t height_;

 public:
  SpriteComponent() {
    memset(static_cast<void *>(this), 0, sizeof(SpriteComponent));
  }
  SpriteComponent(int16_t _textureID, const AM::fb::Rect &_posInTexture, int32_t _width, int32_t _height)
      : textureID_(flatbuffers::EndianScalar(_textureID)),
        padding0__(0),
        posInTexture_(_posInTexture),
        width_(flatbuffers::EndianScalar(_width)),
        height_(flatbuffers::EndianScalar(_height)) {
    (void)padding0__;
  }
  int16_t textureID() const {
    return flatbuffers::EndianScalar(textureID_);
  }
  const AM::fb::Rect &posInTexture() const {
    return posInTexture_;
  }
  int32_t width() const {
    return flatbuffers::EndianScalar(width_);
  }
  int32_t height() const {
    return flatbuffers::EndianScalar(height_);
  }
};
FLATBUFFERS_STRUCT_END(SpriteComponent, 28);

struct ConnectionRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ConnectionRequestBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PLAYERNAME = 4
  };
  const flatbuffers::String *playerName() const {
    return GetPointer<const flatbuffers::String *>(VT_PLAYERNAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_PLAYERNAME) &&
           verifier.VerifyString(playerName()) &&
           verifier.EndTable();
  }
};

struct ConnectionRequestBuilder {
  typedef ConnectionRequest Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_playerName(flatbuffers::Offset<flatbuffers::String> playerName) {
    fbb_.AddOffset(ConnectionRequest::VT_PLAYERNAME, playerName);
  }
  explicit ConnectionRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ConnectionRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ConnectionRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<ConnectionRequest> CreateConnectionRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> playerName = 0) {
  ConnectionRequestBuilder builder_(_fbb);
  builder_.add_playerName(playerName);
  return builder_.Finish();
}

struct ConnectionRequest::Traits {
  using type = ConnectionRequest;
  static auto constexpr Create = CreateConnectionRequest;
};

inline flatbuffers::Offset<ConnectionRequest> CreateConnectionRequestDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *playerName = nullptr) {
  auto playerName__ = playerName ? _fbb.CreateString(playerName) : 0;
  return AM::fb::CreateConnectionRequest(
      _fbb,
      playerName__);
}

struct ConnectionResponse FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ConnectionResponseBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CURRENTTICK = 4,
    VT_ENTITYID = 6,
    VT_X = 8,
    VT_Y = 10
  };
  uint32_t currentTick() const {
    return GetField<uint32_t>(VT_CURRENTTICK, 0);
  }
  uint32_t entityID() const {
    return GetField<uint32_t>(VT_ENTITYID, 0);
  }
  float x() const {
    return GetField<float>(VT_X, 0.0f);
  }
  float y() const {
    return GetField<float>(VT_Y, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_CURRENTTICK) &&
           VerifyField<uint32_t>(verifier, VT_ENTITYID) &&
           VerifyField<float>(verifier, VT_X) &&
           VerifyField<float>(verifier, VT_Y) &&
           verifier.EndTable();
  }
};

struct ConnectionResponseBuilder {
  typedef ConnectionResponse Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_currentTick(uint32_t currentTick) {
    fbb_.AddElement<uint32_t>(ConnectionResponse::VT_CURRENTTICK, currentTick, 0);
  }
  void add_entityID(uint32_t entityID) {
    fbb_.AddElement<uint32_t>(ConnectionResponse::VT_ENTITYID, entityID, 0);
  }
  void add_x(float x) {
    fbb_.AddElement<float>(ConnectionResponse::VT_X, x, 0.0f);
  }
  void add_y(float y) {
    fbb_.AddElement<float>(ConnectionResponse::VT_Y, y, 0.0f);
  }
  explicit ConnectionResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ConnectionResponse> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ConnectionResponse>(end);
    return o;
  }
};

inline flatbuffers::Offset<ConnectionResponse> CreateConnectionResponse(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t currentTick = 0,
    uint32_t entityID = 0,
    float x = 0.0f,
    float y = 0.0f) {
  ConnectionResponseBuilder builder_(_fbb);
  builder_.add_y(y);
  builder_.add_x(x);
  builder_.add_entityID(entityID);
  builder_.add_currentTick(currentTick);
  return builder_.Finish();
}

struct ConnectionResponse::Traits {
  using type = ConnectionResponse;
  static auto constexpr Create = CreateConnectionResponse;
};

struct InputComponent FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef InputComponentBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INPUTSTATES = 4
  };
  const flatbuffers::Vector<AM::fb::InputState> *inputStates() const {
    return GetPointer<const flatbuffers::Vector<AM::fb::InputState> *>(VT_INPUTSTATES);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_INPUTSTATES) &&
           verifier.VerifyVector(inputStates()) &&
           verifier.EndTable();
  }
};

struct InputComponentBuilder {
  typedef InputComponent Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_inputStates(flatbuffers::Offset<flatbuffers::Vector<AM::fb::InputState>> inputStates) {
    fbb_.AddOffset(InputComponent::VT_INPUTSTATES, inputStates);
  }
  explicit InputComponentBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<InputComponent> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<InputComponent>(end);
    return o;
  }
};

inline flatbuffers::Offset<InputComponent> CreateInputComponent(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<AM::fb::InputState>> inputStates = 0) {
  InputComponentBuilder builder_(_fbb);
  builder_.add_inputStates(inputStates);
  return builder_.Finish();
}

struct InputComponent::Traits {
  using type = InputComponent;
  static auto constexpr Create = CreateInputComponent;
};

inline flatbuffers::Offset<InputComponent> CreateInputComponentDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<AM::fb::InputState> *inputStates = nullptr) {
  auto inputStates__ = inputStates ? _fbb.CreateVector<AM::fb::InputState>(*inputStates) : 0;
  return AM::fb::CreateInputComponent(
      _fbb,
      inputStates__);
}

struct PositionComponent FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PositionComponentBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_X = 4,
    VT_Y = 6
  };
  float x() const {
    return GetField<float>(VT_X, 0.0f);
  }
  float y() const {
    return GetField<float>(VT_Y, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_X) &&
           VerifyField<float>(verifier, VT_Y) &&
           verifier.EndTable();
  }
};

struct PositionComponentBuilder {
  typedef PositionComponent Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_x(float x) {
    fbb_.AddElement<float>(PositionComponent::VT_X, x, 0.0f);
  }
  void add_y(float y) {
    fbb_.AddElement<float>(PositionComponent::VT_Y, y, 0.0f);
  }
  explicit PositionComponentBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<PositionComponent> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<PositionComponent>(end);
    return o;
  }
};

inline flatbuffers::Offset<PositionComponent> CreatePositionComponent(
    flatbuffers::FlatBufferBuilder &_fbb,
    float x = 0.0f,
    float y = 0.0f) {
  PositionComponentBuilder builder_(_fbb);
  builder_.add_y(y);
  builder_.add_x(x);
  return builder_.Finish();
}

struct PositionComponent::Traits {
  using type = PositionComponent;
  static auto constexpr Create = CreatePositionComponent;
};

struct MovementComponent FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MovementComponentBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VELX = 4,
    VT_VELY = 6,
    VT_MAXVELX = 8,
    VT_MAXVELY = 10
  };
  float velX() const {
    return GetField<float>(VT_VELX, 0.0f);
  }
  float velY() const {
    return GetField<float>(VT_VELY, 0.0f);
  }
  float maxVelX() const {
    return GetField<float>(VT_MAXVELX, 0.0f);
  }
  float maxVelY() const {
    return GetField<float>(VT_MAXVELY, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_VELX) &&
           VerifyField<float>(verifier, VT_VELY) &&
           VerifyField<float>(verifier, VT_MAXVELX) &&
           VerifyField<float>(verifier, VT_MAXVELY) &&
           verifier.EndTable();
  }
};

struct MovementComponentBuilder {
  typedef MovementComponent Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_velX(float velX) {
    fbb_.AddElement<float>(MovementComponent::VT_VELX, velX, 0.0f);
  }
  void add_velY(float velY) {
    fbb_.AddElement<float>(MovementComponent::VT_VELY, velY, 0.0f);
  }
  void add_maxVelX(float maxVelX) {
    fbb_.AddElement<float>(MovementComponent::VT_MAXVELX, maxVelX, 0.0f);
  }
  void add_maxVelY(float maxVelY) {
    fbb_.AddElement<float>(MovementComponent::VT_MAXVELY, maxVelY, 0.0f);
  }
  explicit MovementComponentBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<MovementComponent> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<MovementComponent>(end);
    return o;
  }
};

inline flatbuffers::Offset<MovementComponent> CreateMovementComponent(
    flatbuffers::FlatBufferBuilder &_fbb,
    float velX = 0.0f,
    float velY = 0.0f,
    float maxVelX = 0.0f,
    float maxVelY = 0.0f) {
  MovementComponentBuilder builder_(_fbb);
  builder_.add_maxVelY(maxVelY);
  builder_.add_maxVelX(maxVelX);
  builder_.add_velY(velY);
  builder_.add_velX(velX);
  return builder_.Finish();
}

struct MovementComponent::Traits {
  using type = MovementComponent;
  static auto constexpr Create = CreateMovementComponent;
};

struct Entity FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef EntityBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_NAME = 6,
    VT_FLAGS = 8,
    VT_INPUTCOMPONENT = 10,
    VT_POSITIONCOMPONENT = 12,
    VT_MOVEMENTCOMPONENT = 14
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  int32_t flags() const {
    return GetField<int32_t>(VT_FLAGS, 0);
  }
  const AM::fb::InputComponent *inputComponent() const {
    return GetPointer<const AM::fb::InputComponent *>(VT_INPUTCOMPONENT);
  }
  const AM::fb::PositionComponent *positionComponent() const {
    return GetPointer<const AM::fb::PositionComponent *>(VT_POSITIONCOMPONENT);
  }
  const AM::fb::MovementComponent *movementComponent() const {
    return GetPointer<const AM::fb::MovementComponent *>(VT_MOVEMENTCOMPONENT);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<int32_t>(verifier, VT_FLAGS) &&
           VerifyOffset(verifier, VT_INPUTCOMPONENT) &&
           verifier.VerifyTable(inputComponent()) &&
           VerifyOffset(verifier, VT_POSITIONCOMPONENT) &&
           verifier.VerifyTable(positionComponent()) &&
           VerifyOffset(verifier, VT_MOVEMENTCOMPONENT) &&
           verifier.VerifyTable(movementComponent()) &&
           verifier.EndTable();
  }
};

struct EntityBuilder {
  typedef Entity Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(Entity::VT_ID, id, 0);
  }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Entity::VT_NAME, name);
  }
  void add_flags(int32_t flags) {
    fbb_.AddElement<int32_t>(Entity::VT_FLAGS, flags, 0);
  }
  void add_inputComponent(flatbuffers::Offset<AM::fb::InputComponent> inputComponent) {
    fbb_.AddOffset(Entity::VT_INPUTCOMPONENT, inputComponent);
  }
  void add_positionComponent(flatbuffers::Offset<AM::fb::PositionComponent> positionComponent) {
    fbb_.AddOffset(Entity::VT_POSITIONCOMPONENT, positionComponent);
  }
  void add_movementComponent(flatbuffers::Offset<AM::fb::MovementComponent> movementComponent) {
    fbb_.AddOffset(Entity::VT_MOVEMENTCOMPONENT, movementComponent);
  }
  explicit EntityBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Entity> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Entity>(end);
    return o;
  }
};

inline flatbuffers::Offset<Entity> CreateEntity(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    int32_t flags = 0,
    flatbuffers::Offset<AM::fb::InputComponent> inputComponent = 0,
    flatbuffers::Offset<AM::fb::PositionComponent> positionComponent = 0,
    flatbuffers::Offset<AM::fb::MovementComponent> movementComponent = 0) {
  EntityBuilder builder_(_fbb);
  builder_.add_movementComponent(movementComponent);
  builder_.add_positionComponent(positionComponent);
  builder_.add_inputComponent(inputComponent);
  builder_.add_flags(flags);
  builder_.add_name(name);
  builder_.add_id(id);
  return builder_.Finish();
}

struct Entity::Traits {
  using type = Entity;
  static auto constexpr Create = CreateEntity;
};

inline flatbuffers::Offset<Entity> CreateEntityDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    const char *name = nullptr,
    int32_t flags = 0,
    flatbuffers::Offset<AM::fb::InputComponent> inputComponent = 0,
    flatbuffers::Offset<AM::fb::PositionComponent> positionComponent = 0,
    flatbuffers::Offset<AM::fb::MovementComponent> movementComponent = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return AM::fb::CreateEntity(
      _fbb,
      id,
      name__,
      flags,
      inputComponent,
      positionComponent,
      movementComponent);
}

struct EntityUpdate FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef EntityUpdateBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CURRENTTICK = 4,
    VT_ENTITIES = 6
  };
  uint32_t currentTick() const {
    return GetField<uint32_t>(VT_CURRENTTICK, 0);
  }
  const flatbuffers::Vector<flatbuffers::Offset<AM::fb::Entity>> *entities() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<AM::fb::Entity>> *>(VT_ENTITIES);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_CURRENTTICK) &&
           VerifyOffset(verifier, VT_ENTITIES) &&
           verifier.VerifyVector(entities()) &&
           verifier.VerifyVectorOfTables(entities()) &&
           verifier.EndTable();
  }
};

struct EntityUpdateBuilder {
  typedef EntityUpdate Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_currentTick(uint32_t currentTick) {
    fbb_.AddElement<uint32_t>(EntityUpdate::VT_CURRENTTICK, currentTick, 0);
  }
  void add_entities(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<AM::fb::Entity>>> entities) {
    fbb_.AddOffset(EntityUpdate::VT_ENTITIES, entities);
  }
  explicit EntityUpdateBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<EntityUpdate> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<EntityUpdate>(end);
    return o;
  }
};

inline flatbuffers::Offset<EntityUpdate> CreateEntityUpdate(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t currentTick = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<AM::fb::Entity>>> entities = 0) {
  EntityUpdateBuilder builder_(_fbb);
  builder_.add_entities(entities);
  builder_.add_currentTick(currentTick);
  return builder_.Finish();
}

struct EntityUpdate::Traits {
  using type = EntityUpdate;
  static auto constexpr Create = CreateEntityUpdate;
};

inline flatbuffers::Offset<EntityUpdate> CreateEntityUpdateDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t currentTick = 0,
    const std::vector<flatbuffers::Offset<AM::fb::Entity>> *entities = nullptr) {
  auto entities__ = entities ? _fbb.CreateVector<flatbuffers::Offset<AM::fb::Entity>>(*entities) : 0;
  return AM::fb::CreateEntityUpdate(
      _fbb,
      currentTick,
      entities__);
}

struct Message FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MessageBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CONTENT_TYPE = 4,
    VT_CONTENT = 6
  };
  AM::fb::MessageContent content_type() const {
    return static_cast<AM::fb::MessageContent>(GetField<uint8_t>(VT_CONTENT_TYPE, 0));
  }
  const void *content() const {
    return GetPointer<const void *>(VT_CONTENT);
  }
  template<typename T> const T *content_as() const;
  const AM::fb::ConnectionRequest *content_as_ConnectionRequest() const {
    return content_type() == AM::fb::MessageContent::ConnectionRequest ? static_cast<const AM::fb::ConnectionRequest *>(content()) : nullptr;
  }
  const AM::fb::ConnectionResponse *content_as_ConnectionResponse() const {
    return content_type() == AM::fb::MessageContent::ConnectionResponse ? static_cast<const AM::fb::ConnectionResponse *>(content()) : nullptr;
  }
  const AM::fb::EntityUpdate *content_as_EntityUpdate() const {
    return content_type() == AM::fb::MessageContent::EntityUpdate ? static_cast<const AM::fb::EntityUpdate *>(content()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_CONTENT_TYPE) &&
           VerifyOffset(verifier, VT_CONTENT) &&
           VerifyMessageContent(verifier, content(), content_type()) &&
           verifier.EndTable();
  }
};

template<> inline const AM::fb::ConnectionRequest *Message::content_as<AM::fb::ConnectionRequest>() const {
  return content_as_ConnectionRequest();
}

template<> inline const AM::fb::ConnectionResponse *Message::content_as<AM::fb::ConnectionResponse>() const {
  return content_as_ConnectionResponse();
}

template<> inline const AM::fb::EntityUpdate *Message::content_as<AM::fb::EntityUpdate>() const {
  return content_as_EntityUpdate();
}

struct MessageBuilder {
  typedef Message Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_content_type(AM::fb::MessageContent content_type) {
    fbb_.AddElement<uint8_t>(Message::VT_CONTENT_TYPE, static_cast<uint8_t>(content_type), 0);
  }
  void add_content(flatbuffers::Offset<void> content) {
    fbb_.AddOffset(Message::VT_CONTENT, content);
  }
  explicit MessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Message> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Message>(end);
    return o;
  }
};

inline flatbuffers::Offset<Message> CreateMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    AM::fb::MessageContent content_type = AM::fb::MessageContent::NONE,
    flatbuffers::Offset<void> content = 0) {
  MessageBuilder builder_(_fbb);
  builder_.add_content(content);
  builder_.add_content_type(content_type);
  return builder_.Finish();
}

struct Message::Traits {
  using type = Message;
  static auto constexpr Create = CreateMessage;
};

inline bool VerifyMessageContent(flatbuffers::Verifier &verifier, const void *obj, MessageContent type) {
  switch (type) {
    case MessageContent::NONE: {
      return true;
    }
    case MessageContent::ConnectionRequest: {
      auto ptr = reinterpret_cast<const AM::fb::ConnectionRequest *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MessageContent::ConnectionResponse: {
      auto ptr = reinterpret_cast<const AM::fb::ConnectionResponse *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MessageContent::EntityUpdate: {
      auto ptr = reinterpret_cast<const AM::fb::EntityUpdate *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyMessageContentVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyMessageContent(
        verifier,  values->Get(i), types->GetEnum<MessageContent>(i))) {
      return false;
    }
  }
  return true;
}

inline const AM::fb::Message *GetMessage(const void *buf) {
  return flatbuffers::GetRoot<AM::fb::Message>(buf);
}

inline const AM::fb::Message *GetSizePrefixedMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<AM::fb::Message>(buf);
}

inline bool VerifyMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<AM::fb::Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<AM::fb::Message>(nullptr);
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<AM::fb::Message> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<AM::fb::Message> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace fb
}  // namespace AM

#endif  // FLATBUFFERS_GENERATED_MESSAGE_AM_FB_H_
