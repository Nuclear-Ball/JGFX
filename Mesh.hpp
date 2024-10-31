#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <stdexcept>
#include <cstring>
#include <glad/glad.h>

#include "pos2d.h"
#include "LowLevel/Common.h"

namespace JGFX {
	enum class AttribType {
		I8 = GL_BYTE,
		UI8 = GL_UNSIGNED_BYTE,
		I16 = GL_SHORT,
		UI16 = GL_UNSIGNED_SHORT,
		I32 = GL_INT,
		UI32 = GL_UNSIGNED_INT,

		FL32 = GL_FLOAT,
		FL64 = GL_DOUBLE
	};

	template<typename T> inline constexpr AttribType AttType() {
		throw std::runtime_error("The type given to AttType() is not supported");
		return AttribType::I8;
	}

	template<> inline constexpr AttribType AttType<int8>() { return AttribType::I8; }
	template<> inline constexpr AttribType AttType<uint8>() { return AttribType::UI8; }
	template<> inline constexpr AttribType AttType<int16>() { return AttribType::I16; }
	template<> inline constexpr AttribType AttType<uint16>() { return AttribType::UI16; }
	template<> inline constexpr AttribType AttType<int32>() { return AttribType::I32; }
	template<> inline constexpr AttribType AttType<uint32>() { return AttribType::UI32; }
	template<> inline constexpr AttribType AttType<fl32>() { return AttribType::FL32; }
	template<> inline constexpr AttribType AttType<fl64>() { return AttribType::FL64; }


	const std::unordered_map<AttribType, size_t> attrib_size_table = {{AttribType::I8,   1},
	                                                                  {AttribType::UI8,  1},
	                                                                  {AttribType::I16,  2},
	                                                                  {AttribType::UI16, 2},
	                                                                  {AttribType::I32,  4},
	                                                                  {AttribType::UI32, 4},
	                                                                  {AttribType::FL32, 4},
	                                                                  {AttribType::FL64, 8}};

	struct RestrictedVertexAttribute {
		AttribType type = AttribType::FL32;
		size_t amount = 1;
		size_t divisor = 0; //Used for instancing. Ignore it, if you are not going to use this. The recommended value for instancing is 1.
		//The more detailed explanation you can find on the internet.
		bool normalize = false;
	};

	struct VertexAttribute : public RestrictedVertexAttribute {
		LLJGFX::HandleType layout_id = 0;

		inline VertexAttribute(LLJGFX::HandleType layout_id = 0,
		                       AttribType type = AttribType::FL32,
		                       size_t amount = 1,
		                       size_t divisor = 0,
		                       bool normalize = false) {
			this->layout_id = layout_id;
			this->type = type;
			this->amount = amount;
			this->divisor = divisor;
			this->normalize = normalize;
		}

		template<typename T> inline VertexAttribute(LLJGFX::HandleType layout_id = 0,
		                       size_t amount = 1,
		                       size_t divisor = 0,
		                       bool normalize = false) {
			this->layout_id = layout_id;
			this->type = AttType<T>();
			this->amount = amount;
			this->divisor = divisor;
			this->normalize = normalize;
		}

		VertexAttribute(LLJGFX::HandleType layout_id, const RestrictedVertexAttribute &attrib) {
			this->layout_id = layout_id;
			type = attrib.type;
			amount = attrib.amount;
			divisor = attrib.divisor;
			normalize = attrib.normalize;
		}
	};

	class VertexLayout {
	private:
		//bool is_instanced = false;
		//In this case we actually need a sorted map for consistency in .CalculateOffset() output (since the order of elements in std::unordered_map is not deterministic)
		std::map<LLJGFX::HandleType, RestrictedVertexAttribute> attribs;
	public:
		typedef std::map<LLJGFX::HandleType, RestrictedVertexAttribute>::iterator map_iter;
		typedef std::pair<const LLJGFX::HandleType, RestrictedVertexAttribute> map_elem;

		inline RestrictedVertexAttribute &operator[](LLJGFX::HandleType layout_id) { return attribs[layout_id]; }
		inline const RestrictedVertexAttribute &operator[](LLJGFX::HandleType layout_id) const { return attribs.find(layout_id)->second; }

		VertexLayout &Add(const VertexAttribute &attrib){
#ifndef NDEBUG
			if (attribs.contains(attrib.layout_id))
				throw std::runtime_error("This layout id is already taken");
#endif
			attribs[attrib.layout_id] = dynamic_cast<const RestrictedVertexAttribute &>(attrib);
			return *this;
		}
		inline VertexLayout &Remove(LLJGFX::HandleType id) {
			attribs.erase(id);
			return *this;
		}
		VertexLayout &Set(const std::vector<VertexAttribute> &data){
			for (const VertexAttribute &i: data)
				Add(i);
			return *this;
		}
		std::vector<VertexAttribute> Get(){
			std::vector<VertexAttribute> res;
			res.reserve(attribs.size());

			for (const map_elem &i: attribs)
				res.push_back(VertexAttribute(i.first, i.second));
			return res;
		}

		inline size_t size() const { return attribs.size(); }
		inline std::map<LLJGFX::HandleType, RestrictedVertexAttribute> &map() { return attribs; }
		inline const std::map<LLJGFX::HandleType, RestrictedVertexAttribute> &map() const { return attribs; }

		inline VertexLayout() = default;
		inline VertexLayout(const std::vector<VertexAttribute> &data) { Set(data); }
		inline VertexLayout(std::initializer_list<VertexAttribute> data) { Set(data); }

		size_t CalculateStride() const {
			size_t stride = 0;
			for (const map_elem &i: attribs)
				stride += attrib_size_table.find(i.second.type)->second * i.second.amount;
			return stride;
		}
		size_t CalculateOffset(LLJGFX::HandleType id){
			const map_iter elem_iter = attribs.find(id);
#ifndef NDEBUG
			if (elem_iter != attribs.end()) {
#endif
				size_t offset = 0;
				for (map_iter i = attribs.begin(); i != elem_iter; i++)
					offset += attrib_size_table.find(i->second.type)->second * i->second.amount;
				return offset;
#ifndef NDEBUG
			} else throw std::runtime_error("Vertex attribute with this layout id does not exist");
#endif
		}
	};

	std::vector<LLJGFX::HandleType> Indexate(size_t vertex_count) {
		std::vector<LLJGFX::HandleType> res(vertex_count);

		for(int i = 0; i < vertex_count; i++)
			res[i] = i;

		return res;
	}
}
namespace LLJGFX {
	namespace VBuff {
		struct Handle {
			HandleType handle;

			bool operator==(Handle oth) const { return handle == oth.handle; }
		};
	}

	namespace VRef {
		struct Handle {
			HandleType handle;

			bool operator==(Handle oth) const { return handle == oth.handle; }
		};
	}
}

template<> struct std::hash<LLJGFX::VBuff::Handle>{
	size_t operator()(LLJGFX::VBuff::Handle handle) const { return handle.handle; }
};
template<> struct std::hash<LLJGFX::VRef::Handle>{
	size_t operator()(LLJGFX::VRef::Handle handle) const { return handle.handle; }
};

namespace LLJGFX {
	namespace Internal{
		struct VBuffDataHolder{
			//bool marked_for_deletion = false;
			std::unordered_set<VRef::Handle> bound_to;
			JGFX::VertexLayout layout;
			//size_t associated_type_hash = 0; //hash of the type, that is associated with the current layout.
			//Used to prevent assignment of data with an invalid type to a vertex buffer
			std::vector<uint8> binary;
		};
		std::unordered_map<VBuff::Handle, VBuffDataHolder> vbuff_data;
		inline VBuffDataHolder& find_vb_data(VBuff::Handle vbuff_handle) {
#ifndef NDEBUG
			if(!vbuff_data.contains(vbuff_handle))
				throw std::runtime_error("The requested vertex buffer does not exists");
#endif
			return vbuff_data.find(vbuff_handle)->second;
		}

		struct VRefDataHolder{
			//bool is_initialized = false;
			HandleType ibuff_handle = INVALID_HANDLE;
			std::vector<HandleType> ibuff_data;
			std::unordered_set<VBuff::Handle> bound_vertex_buffers;
		};
		std::unordered_map<VRef::Handle, VRefDataHolder> vref_data;
		inline VRefDataHolder& find_vr_data(VRef::Handle vref_handle) {
#ifndef NDEBUG
			if(!vref_data.contains(vref_handle))
				throw std::runtime_error("The requested vertex referencer does not exists");
#endif
			return vref_data.find(vref_handle)->second;
		}

		inline void UnbindBuffers(){
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		inline void UnbindVertexLayout(VRef::Handle vao, VBuff::Handle vbo){
			UnbindBuffers();
			glBindVertexArray(vao.handle);
			glBindBuffer(GL_ARRAY_BUFFER, vbo.handle);

			const VBuffDataHolder& vb_dat = find_vb_data(vbo);

			for(const std::pair<HandleType, JGFX::RestrictedVertexAttribute> i : vb_dat.layout.map())
				{ glVertexAttribDivisor(i.first, 0); glDisableVertexAttribArray(i.first); }
			glBindVertexArray(0);
		}
		void BindVertexLayout(VRef::Handle vao, VBuff::Handle vbo, const JGFX::VertexLayout& layout){
			Internal::UnbindBuffers();
			glBindVertexArray(vao.handle);
			glBindBuffer(GL_ARRAY_BUFFER, vbo.handle);

			size_t offset = 0;
			const size_t stride = layout.CalculateStride();
			for(const JGFX::VertexLayout::map_elem& elem : layout.map()){
				glEnableVertexAttribArray(elem.first);
				glVertexAttribPointer(elem.first,
				                      (GLint)elem.second.amount,
				                      (GLint)elem.second.type,
				                      elem.second.normalize,
				                      (GLsizei)stride,
				                      (void*)offset);

				glVertexAttribDivisor(elem.first, elem.second.divisor);

				offset += JGFX::attrib_size_table.find(elem.second.type)->second * elem.second.amount;
			}
			glBindVertexArray(0);
		}
	}

	namespace VBuff{
		template<typename T> inline void ForceSetData(Handle vbuff_handle, const T* data, size_t size){
			Internal::VBuffDataHolder& dat = Internal::find_vb_data(vbuff_handle);
			glBindBuffer(GL_ARRAY_BUFFER, vbuff_handle.handle);
			glBufferData(GL_ARRAY_BUFFER, sizeof(T) * size, data, GL_STATIC_DRAW);

			dat.binary.resize(size * sizeof(T));
			memcpy(dat.binary.data(), data, size * sizeof(T));
		}
		template<typename T> inline void ForceSetData(Handle vbuff_handle, const std::vector<T>& data)
			{ ForceSetData(vbuff_handle, data.data(), data.size()); }

		template<typename T> inline void SetData(Handle vbuff_handle, const T* data, size_t size){
			Internal::VBuffDataHolder& dat = Internal::find_vb_data(vbuff_handle);

			if(dat.layout.size())
				if((size * sizeof(T)) % dat.layout.CalculateStride())
					throw std::runtime_error("The provided data to VertexBuffer::SetData() does not match the layout");

			ForceSetData(vbuff_handle, data, size);
		}
		template<typename T> inline void SetData(Handle vbuff_handle, const std::vector<T>& data)
			{ SetData(vbuff_handle, data.data(), data.size()); }

		inline size_t GetSize(Handle vbuff_handle){
			Internal::VBuffDataHolder& dat = Internal::find_vb_data(vbuff_handle);
			return dat.binary.size() / dat.layout.CalculateStride();
		}
		inline size_t GetBinarySize(Handle vbuff_handle)
			{ return Internal::find_vb_data(vbuff_handle).binary.size(); }
		template<typename T> inline void GetData(Handle vbuff_handle, T* dest){
			Internal::VBuffDataHolder& dat = Internal::find_vb_data(vbuff_handle);
			//if(typeid(T).hash_code() != dat.associated_type_hash && dat.associated_type_hash != 0)
			//	throw std::runtime_error("Trying to assign data with invalid type to a vertex buffer(make sure it matches what was described in the vertex layout)");
			memcpy(dest, dat.binary.data(), dat.binary.size());
		}
		template<typename T> inline std::vector<T> GetData(Handle vbuff_handle){
			std::vector<T> res(GetBinarySize(vbuff_handle));
			GetData(vbuff_handle, res.data());
			return res;
		}

		void SetLayout(Handle vbuff_handle, const JGFX::VertexLayout& layout) {
			Internal::VBuffDataHolder& dat = Internal::find_vb_data(vbuff_handle);

			for(const VRef::Handle i : dat.bound_to){
				Internal::UnbindVertexLayout(i, vbuff_handle);
				Internal::BindVertexLayout(i, vbuff_handle, layout);
			}
			dat.layout = layout;
		}
		inline JGFX::VertexLayout GetLayout(Handle vbuff_handle)
			{ return Internal::find_vb_data(vbuff_handle).layout; }

		inline Handle Make(){
			Handle handle;
			glGenBuffers(1, &handle.handle);
			Internal::vbuff_data[handle];

#ifdef JGFX_LL_DEBUG_OBJECTS_TRACKING
			std::cout << "buff creation called: " << handle.handle << "\n";
#endif
			return handle;
		}
		template<typename T> inline Handle Make(const T* data, size_t size, const JGFX::VertexLayout& layout){
			Handle handle = Make();
			SetLayout(handle, layout);
			SetData(handle, data, size);
			return handle;
		}
		template<typename T> inline Handle Make(const std::vector<T>& data, const JGFX::VertexLayout& layout)
			{ return Make(data.data(), data.size(), layout); }
		inline void Delete(Handle vbuff_handle) {
			if(vbuff_handle.handle != INVALID_HANDLE){
				Internal::VBuffDataHolder& vb_dat = Internal::find_vb_data(vbuff_handle);

				for(const VRef::Handle i : vb_dat.bound_to){
					Internal::VRefDataHolder& vr_dat = Internal::find_vr_data(i);

					Internal::UnbindVertexLayout(i, vbuff_handle);
					vr_dat.bound_vertex_buffers.erase(vbuff_handle);
				}

				glDeleteBuffers(1, &vbuff_handle.handle);
				Internal::vbuff_data.erase(vbuff_handle);

#ifdef JGFX_LL_DEBUG_OBJECTS_TRACKING
				std::cout << "buff deletion called: " << vbuff_handle.handle << "\n";
#endif
			}
		}
	}

	namespace VRef{
		inline void AttachVBuff(Handle vref_handle, VBuff::Handle vbuff_handle){
			Internal::VRefDataHolder& vr_dat = Internal::find_vr_data(vref_handle);
			Internal::VBuffDataHolder& vb_dat = Internal::find_vb_data(vbuff_handle);

			Internal::BindVertexLayout(vref_handle, vbuff_handle, vb_dat.layout);

			vb_dat.bound_to.insert(vref_handle);
			vr_dat.bound_vertex_buffers.insert(vbuff_handle);

			//Bind the buffer
			//Apply layout
			//Add dependency to the buffer data
		}
		inline void AttachVBuffs(Handle vref_handle, const std::vector<VBuff::Handle>& vbuffs){
			for(const VBuff::Handle i : vbuffs)
				AttachVBuff(vref_handle, i);
		}
		inline void DetachVBuff(Handle vref_handle, VBuff::Handle vbuff_handle) {
			Internal::VRefDataHolder& vr_dat = Internal::find_vr_data(vref_handle);

			if(!vr_dat.bound_vertex_buffers.contains(vbuff_handle))
				throw std::runtime_error("The given buffer is not even bound to vertex referencer");

			Internal::VBuffDataHolder& vb_dat = Internal::find_vb_data(vbuff_handle);
			Internal::UnbindVertexLayout(vref_handle, vbuff_handle);

			vr_dat.bound_vertex_buffers.erase(vbuff_handle);
			vb_dat.bound_to.erase(vref_handle);

			//Check, if it's even bound
			//Unbind the layout
			//Remove any references to both objects
		}
		inline std::vector<VBuff::Handle> GetBoundVBuffs(Handle vref_handle){
			Internal::VRefDataHolder& dat = Internal::find_vr_data(vref_handle);
			std::vector<VBuff::Handle> res;
			res.reserve(dat.bound_vertex_buffers.size());

			for(const VBuff::Handle i : dat.bound_vertex_buffers)
				res.push_back(i);

			return res;
		}

		inline void SetIndexingData(Handle vref_handle, const HandleType* data, size_t size) {
			Internal::VRefDataHolder& dat = Internal::find_vr_data(vref_handle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dat.ibuff_handle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(HandleType), data, GL_STATIC_DRAW);

			dat.ibuff_data.resize(size);
			memcpy(dat.ibuff_data.data(), data, size * sizeof(HandleType));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		inline void SetIndexingData(Handle vref_handle, const std::vector<HandleType>& data)
			{ SetIndexingData(vref_handle, data.data(), data.size()); }
		inline std::vector<HandleType> GetIndexingData(Handle vref_handle)
			{ return Internal::find_vr_data(vref_handle).ibuff_data; }

		inline Handle Make() {
			Handle handle;
			glGenVertexArrays(1, &handle.handle);

			Internal::VRefDataHolder& dat = Internal::vref_data[handle];
			glGenBuffers(1, &dat.ibuff_handle);

			glBindVertexArray(handle.handle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dat.ibuff_handle);

			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#ifdef JGFX_LL_DEBUG_OBJECTS_TRACKING
			std::cout << "ref creation called: " << handle.handle << "\n";
#endif
			return handle;
		}
		inline Handle Make(const std::vector<VBuff::Handle>& vbuffs, const std::vector<HandleType>& indexing_data){
			Handle handle = Make();

			AttachVBuffs(handle, vbuffs);
			SetIndexingData(handle, indexing_data);

			return handle;
		}
		inline void Delete(Handle vref_handle) {
			if(vref_handle.handle != INVALID_HANDLE) {
#ifdef JGFX_LL_DEBUG_OBJECTS_TRACKING
				std::cout << "ref deletion called: " << vref_handle.handle << "\n";
#endif

				Internal::VRefDataHolder &dat = Internal::find_vr_data(vref_handle);

				const std::unordered_set<VBuff::Handle> pls_dont_segfault_again = dat.bound_vertex_buffers;
				for (const VBuff::Handle i : pls_dont_segfault_again) {
					Internal::VBuffDataHolder &vb_dat = Internal::find_vb_data(i);
					//vb_dat.bound_to.erase(vref_handle);

					DetachVBuff(vref_handle, i);
				}

				glDeleteVertexArrays(1, &vref_handle.handle);
				glDeleteBuffers(1, &dat.ibuff_handle);
				Internal::vref_data.erase(vref_handle);
			}
			//Remove references from the bound vertex buffers
			//If vertex buffers are marked for deletion, and if they are not bound to anything anymore, delete them
			//Delete vao, ebo
		}
	}
}

namespace JGFX {
	class VertexBuffer{
	protected:
		LLJGFX::VBuff::Handle vbo_handle = { LLJGFX::INVALID_HANDLE };
	public:
		template <typename T> inline VertexBuffer& SetData(const T* data, size_t size)
			{ LLJGFX::VBuff::SetData(vbo_handle, data, size); return *this; }
		template <typename T> inline VertexBuffer& SetData(const std::vector<T>& data)
			{ SetData(data.data(), data.size()); return *this; }

		inline VertexBuffer& SetLayout(const VertexLayout& layout)
			{ LLJGFX::VBuff::SetLayout(vbo_handle, layout); return *this; }

		//Getter functions
		template <typename T> inline std::vector<T> data() const { return LLJGFX::VBuff::GetData<T>(vbo_handle); }
		inline VertexLayout layout() const { return LLJGFX::VBuff::GetLayout(vbo_handle); }
		inline LLJGFX::VBuff::Handle handle() const { return vbo_handle; }
		inline size_t size() const { return LLJGFX::VBuff::GetSize(vbo_handle); }

		inline VertexBuffer& Init() {
			this->~VertexBuffer();
			vbo_handle = LLJGFX::VBuff::Make();
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb was constructed " << vbo_handle.handle << "\n";
#endif
			return *this;
		}
		template <typename T> inline VertexBuffer& Init(const T* data, size_t size, const JGFX::VertexLayout& layout) {
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb called construction with raw data: ";
#endif
			Init();
			LLJGFX::VBuff::SetLayout(vbo_handle, layout);
			LLJGFX::VBuff::SetData(vbo_handle, data, size);

			return *this;
		}
		template <typename T> inline VertexBuffer& Init(const std::vector<T>& data, const JGFX::VertexLayout& layout)
			{ Init(data.data(), data.size(), layout); return *this; }

		//Stop object from initializing
		inline explicit VertexBuffer(bool) {}

		inline VertexBuffer() {
			Init();
		}
		inline explicit VertexBuffer(LLJGFX::VBuff::Handle handle) {
			vbo_handle = handle;

#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb was constructed from raw handle " << vbo_handle.handle << "\n";
#endif
		}
		template<typename T> inline VertexBuffer(const std::vector<T>& data, const JGFX::VertexLayout& layout)
			{ Init(data, layout); }
		template<typename T> inline VertexBuffer(const T* data, size_t size, const JGFX::VertexLayout& layout)
			{ Init(data, size, layout); }

		inline VertexBuffer& operator=(const VertexBuffer& cpy) {
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb copy operator was called " << vbo_handle.handle << "\n";
#endif
			SetLayout(cpy.layout()).SetData(cpy.data<uint8>()); return *this; }
		inline VertexBuffer(const VertexBuffer& cpy) {
			Init();
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "rel: ";
#endif
			operator=(cpy);
		}

		inline VertexBuffer& operator=(VertexBuffer&& cpy) noexcept {
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb move operator called " << vbo_handle.handle << " <- " << cpy.vbo_handle.handle << ": ";
#endif
			this->~VertexBuffer();
			std::swap(vbo_handle, cpy.vbo_handle);

			return *this;
		}
		inline VertexBuffer(VertexBuffer&& cpy) noexcept {
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb was constructed: ";
#endif
			operator=(std::move(cpy));
		}

		inline ~VertexBuffer(){
#ifdef JGFX_DEBUG_OBJECT_TRACKING
			std::cout << "vb was destroyed " << vbo_handle.handle << "\n";
#endif
			LLJGFX::VBuff::Delete(vbo_handle);
			vbo_handle = { LLJGFX::INVALID_HANDLE };
		}
	};
	class VertexReferencer {
	protected:
		LLJGFX::VRef::Handle vr_handle = { LLJGFX::INVALID_HANDLE };
	public:
		inline VertexReferencer& SetIndexingData(const LLJGFX::HandleType* data, size_t size)
			{ LLJGFX::VRef::SetIndexingData(vr_handle, data, size); return *this; }
		inline VertexReferencer& SetIndexingData(const std::vector<LLJGFX::HandleType>& data)
			{ SetIndexingData(data.data(), data.size()); return *this; }

		inline VertexReferencer& AttachVertexBuffer(LLJGFX::VBuff::Handle vbuff)
			{ LLJGFX::VRef::AttachVBuff(vr_handle, vbuff); return *this; }
		inline VertexReferencer& AttachVertexBuffer(const VertexBuffer& vbuff)
			{ AttachVertexBuffer(vbuff.handle()); return *this; }
		inline VertexReferencer& AttachVertexBuffers(const std::vector<LLJGFX::VBuff::Handle>& vbuffs) {
			for(const LLJGFX::VBuff::Handle i : vbuffs)
				AttachVertexBuffer(i);
			return *this;
		}

		inline VertexReferencer& DetachVertexBuffer(LLJGFX::VBuff::Handle vbuff)
			{ LLJGFX::VRef::DetachVBuff(vr_handle, vbuff); return *this; }
		inline VertexReferencer& DetachVertexBuffer(const VertexBuffer& vbuff)
			{ DetachVertexBuffer(vbuff.handle()); return *this; }

		//Getters
		inline std::vector<LLJGFX::HandleType> indexing_data() const
		{ return LLJGFX::VRef::GetIndexingData(vr_handle); }
		inline std::vector<LLJGFX::VBuff::Handle> attached_buffers() const
			{ return LLJGFX::VRef::GetBoundVBuffs(vr_handle); };
		inline LLJGFX::VRef::Handle handle() const { return vr_handle; }

		inline VertexReferencer& Init() {
			this->~VertexReferencer();
			vr_handle = LLJGFX::VRef::Make();
			return *this;
		}
		inline VertexReferencer& Init(const std::vector<LLJGFX::VBuff::Handle>& vbuffs,
									  const std::vector<LLJGFX::HandleType>& indexing_data) {
			Init();
			LLJGFX::VRef::AttachVBuffs(vr_handle, vbuffs);
			LLJGFX::VRef::SetIndexingData(vr_handle, indexing_data);

			return *this;
		}
		inline VertexReferencer& Init(const JGFX::VertexBuffer& vbuff,
									  const std::vector<LLJGFX::HandleType>& indexing_data)
			{ Init({vbuff.handle()}, indexing_data); return *this; }

		inline explicit VertexReferencer(bool){}
		inline VertexReferencer() { Init(); }
		inline VertexReferencer(const std::vector<LLJGFX::VBuff::Handle>& vbuffs,
								const std::vector<LLJGFX::HandleType>& indexing_data)
			{ Init(vbuffs, indexing_data); }
		inline VertexReferencer(const JGFX::VertexBuffer& vbuff, const std::vector<LLJGFX::HandleType>& indexing_data)
			{ Init(vbuff, indexing_data); }

		inline VertexReferencer& operator=(const VertexReferencer& cpy){
			SetIndexingData(cpy.indexing_data());

			const std::vector<LLJGFX::VBuff::Handle> attached = cpy.attached_buffers();
			for(const LLJGFX::VBuff::Handle i : attached)
				AttachVertexBuffer(i);

			return *this;
		}
		inline VertexReferencer(const VertexReferencer& cpy){
			Init();
			operator=(cpy);
		}

		inline VertexReferencer& operator=(VertexReferencer&& cpy) noexcept {
			this->~VertexReferencer();
			std::swap(vr_handle, cpy.vr_handle);
			return *this;
		}
		inline VertexReferencer(VertexReferencer&& cpy) noexcept { operator=(std::move(cpy)); }

		inline ~VertexReferencer(){
			LLJGFX::VRef::Delete(vr_handle);
			vr_handle = { LLJGFX::INVALID_HANDLE };
		}
	};

	class VertexCollection{
	public:
		class RestrictedVertexBuffer : public VertexBuffer {
		private:
			inline RestrictedVertexBuffer& Init() = delete;
			template <typename T> inline RestrictedVertexBuffer& Init(const std::vector<T>& data,
																	  const JGFX::VertexLayout& layout) = delete;
			template <typename T> inline RestrictedVertexBuffer& Init(const T* data, size_t size,
																	  const JGFX::VertexLayout& layout) = delete;

			inline RestrictedVertexBuffer& operator=(RestrictedVertexBuffer&& cpy) = delete;
			//inline RestrictedVertexBuffer(RestrictedVertexBuffer&& cpy) = delete;
		};

		LLJGFX::VRef::Handle handle_ = {LLJGFX::INVALID_HANDLE };
		std::unordered_map<std::string, VertexBuffer> bound_buffers;
	public:
		//Add or remove buffers
		inline VertexCollection& AddBuffer(VertexBuffer&& dat, const std::string& name){
#ifndef NDEBUG
			if(bound_buffers.contains(name))
				throw std::runtime_error("This name for a vertex buffer is already taken");
#endif
			//bound_buffers[name] = std::move(dat);
			bound_buffers.insert({name, std::move(dat)});
			LLJGFX::VRef::AttachVBuff(handle_, bound_buffers[name].handle());

			return *this;
		}
		inline VertexCollection& AddBuffer(LLJGFX::VBuff::Handle vbo_handle, const std::string& name)
			{ AddBuffer(VertexBuffer{vbo_handle}, name); return *this; }
		inline VertexBuffer RemoveBuffer(const std::string& name){
#ifndef NDEBUG
			if(!bound_buffers.contains(name))
				throw std::runtime_error("The vertex buffer with this name does not exists");
#endif
			VertexBuffer target = std::move(bound_buffers[name]);
			LLJGFX::VRef::DetachVBuff(handle_, target.handle());
			bound_buffers.erase(name);

			return target;
		}

		//Rename buffer
		inline VertexCollection& RenameBuffer(const std::string& old_name, const std::string& new_name){
#ifndef NDEBUG
			if(!bound_buffers.contains(old_name))
				throw std::runtime_error("The vertex buffer with this name does not exists");
			if(bound_buffers.contains(new_name))
				throw std::runtime_error("This name for a vertex buffer is already taken");
#endif
			bound_buffers.insert({new_name, std::move(bound_buffers[old_name])});
			bound_buffers.erase(old_name);

			return *this;
		}

		//Get buffer
		RestrictedVertexBuffer& operator[](const std::string& name){
#ifndef NDEBUG
			if(!bound_buffers.contains(name))
				throw std::runtime_error("The vertex buffer with this name does not exists");
#endif
			return static_cast<RestrictedVertexBuffer&>(bound_buffers[name]);
		}

		//Set indexing
		inline VertexCollection& SetIndexingData(const LLJGFX::HandleType* data, size_t size)
			{ LLJGFX::VRef::SetIndexingData(handle_, data, size); return *this; }
		inline VertexCollection& SetIndexingData(const std::vector<LLJGFX::HandleType>& data)
			{ SetIndexingData(data.data(), data.size()); return *this; }

		//Get indexing
		inline std::vector<LLJGFX::HandleType> indexing_data() const
			{ return LLJGFX::VRef::GetIndexingData(handle_); }

		//Initialization
		inline VertexCollection& Init() {
			this->~VertexCollection();
			handle_ = LLJGFX::VRef::Make();
			return *this;
		}
		inline VertexCollection& Init(VertexBuffer&& vbuff, const std::string& vb_name,
									  const std::vector<LLJGFX::HandleType>& indexing_data){
			Init();
			AddBuffer(std::move(vbuff), vb_name);
			SetIndexingData(indexing_data);
			return *this;
		}

		inline LLJGFX::VRef::Handle handle() const { return handle_; }

		//Constructors
		explicit VertexCollection(bool){}
		VertexCollection(){ Init(); }
		VertexCollection(VertexBuffer&& vbuff, const std::string& vb_name,
						 const std::vector<LLJGFX::HandleType>& indexing_data)
						 { Init(std::move(vbuff), vb_name, indexing_data); }

		//Copy and move operators/constructors
		VertexCollection& operator=(const VertexCollection& cpy){
			bound_buffers = cpy.bound_buffers;

			//Attach copied buffers to vertex referencer
			for(const std::pair<const std::string, VertexBuffer>& i : bound_buffers)
				LLJGFX::VRef::AttachVBuff(handle_, i.second.handle());
			LLJGFX::VRef::SetIndexingData(handle_, cpy.indexing_data());

			return *this;
		}
		inline VertexCollection(const VertexCollection& cpy) { Init(); operator=(cpy); }

		inline VertexCollection& operator=(VertexCollection&& cpy) noexcept {
			this->~VertexCollection();

			std::swap(handle_, cpy.handle_);

			return *this;
		}
		inline VertexCollection(VertexCollection&& cpy) noexcept { operator=(std::move(cpy)); }

		//Destructor
		~VertexCollection(){
			LLJGFX::VRef::Delete(handle_);
			handle_ = {LLJGFX::INVALID_HANDLE };
		}
	};

	/*
	class Mesh{
	private:
		class RestrictedVertexBuffer{
		private:
			Raw::HandleType vbo_handle = Raw::INVALID_HANDLE;
			size_t binary_size = 0;
			VertexLayout layout;
			size_t type_hash = 0;
			size_t type_size = 0;
		public:
			template <typename T> inline void SetData(const T* data, size_t size) {
				if(typeid(T).hash_code() != type_hash)
					throw std::runtime_error("Type that was given to .SetData() does not match with initialized type");
				Raw::SetVboData(data, size);
			}
			template <typename T> inline void SetData(const std::vector<T>& data) {
				if(typeid(T).hash_code() != type_hash)
					throw std::runtime_error("Type that was given to .SetData() does not match with initialized type");
				Raw::SetVboData(data);
			}
		};
	public:

	};*/
}