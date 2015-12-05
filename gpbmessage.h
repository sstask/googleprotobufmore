#ifndef _GPBMESSAGE_H_
#define _GPBMESSAGE_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <string>
#include <map>
#include <set>
#include <iostream>

#define SAFE_DELETE(p)	if((p)) {delete (p); (p) = NULL;}

class gpberror : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
	gpberror(){}
	~gpberror(){}

	virtual void AddError(const std::string& filename, int line, int column,
		const std::string& message){
			std::cerr<<"error: file="<<filename<<" at line "<<line<<" column "<<column<<std::endl<<"err="<<message<<std::endl;
	}
};

class gpbmessage;
class CGpbdata
{
private:
	CGpbdata();
	~CGpbdata();

public:
	static CGpbdata* getinstance() {if(mInstance == NULL) mInstance=new CGpbdata();return mInstance;}
	static void delinstance() {if(mInstance){mInstance->destory();SAFE_DELETE(mInstance);}}
private:
	static CGpbdata* mInstance;

public:
	bool loadproto(const char* path, const char* file);//it also load the proto files which para file 'import'

	bool newmessage(gpbmessage& msg, const char* msgname);
	bool getmessagefromstring(gpbmessage& msg, const char* msgname, const std::string& content);

	google::protobuf::DynamicMessageFactory* getfactory() {return mFactory;}
	const google::protobuf::FieldDescriptor* getfileddescriptor(const char* msgname, const char* fieldname)
	{return mMessageFieldMap[std::string(msgname)+"+"+fieldname];}

	void destory();
protected:
	const google::protobuf::Message* getmessage(const char* msgname);

private:
	typedef std::map<std::string, const google::protobuf::Message*> GPBMESSAGEMAP;
	typedef std::map<std::string, const google::protobuf::FieldDescriptor*> GPBFIELDMAP;

	google::protobuf::compiler::Importer* mImporter;
	google::protobuf::DynamicMessageFactory* mFactory;
	gpberror mError;

	GPBMESSAGEMAP mMessageMap;
	GPBFIELDMAP mMessageFieldMap;

	int mMessageCount;
public:
	void add() {mMessageCount++;}
	void sub() {mMessageCount--;}
};


class gpbmessage;
class Usable_lock		//no inherit
{
	friend class gpbmessage;
private:
	Usable_lock() {}
	Usable_lock(const Usable_lock&) {}
};

class gpbmessage: public virtual Usable_lock //no inherit
{
public:
	gpbmessage();
	gpbmessage(const char* mname);
	gpbmessage(const char* mname, const std::string& content);
	~gpbmessage();

public:
	static bool loadproto(const char* path, const char* file);
	static void destoryproto();

public:
	bool copyfrom(const gpbmessage& copy);
	bool serializetostring(std::string* output);
	bool parsefromstring(const char* mname, const char* content);
	std::string shortdebugstring();

public:
#define HANDLE_FIELD google::protobuf::Message* message = get();														\
	if(message == NULL || mGpbData == NULL)																				\
	return false;																										\
	const google::protobuf::Reflection *reflection = message->GetReflection();											\
	if(reflection == NULL)																								\
	return false;																										\
	const google::protobuf::FieldDescriptor* field = mGpbData->getfileddescriptor(message->GetTypeName().c_str(), fieldname);	\
	if(field == NULL && message->GetDescriptor())																		\
	field = message->GetDescriptor()->FindFieldByName(fieldname);														\
	if(field == NULL)																									\
	return false

	template<typename Type>
	bool getval(const char* fieldname, Type& val)
	{
		HANDLE_FIELD;
		if(field->is_repeated() || field->is_map())
			return false;

		switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, FUNCASE)														\
		case google::protobuf::FieldDescriptor::CPPTYPE_##UPPERCASE :						\
		val = reflection->Get##FUNCASE(*message, field);									\
		break

			HANDLE_TYPE( INT32, Int32);
			HANDLE_TYPE( INT64, Int64);
			HANDLE_TYPE(UINT32, UInt32);
			HANDLE_TYPE(UINT64, UInt64);
			HANDLE_TYPE(DOUBLE, Double);
			HANDLE_TYPE( FLOAT, Float);
			HANDLE_TYPE(  BOOL, Bool);
			HANDLE_TYPE(  ENUM, EnumValue);
#undef HANDLE_TYPE

		default:
			return false;
		}
		return true;
	}

	template<>
	bool getval(const char* fieldname, std::string& val)
	{
		HANDLE_FIELD;
		if(field->is_repeated() || field->is_map())
			return false;

		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
			{
				val = reflection->GetString(*message, field);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<>
	bool getval(const char* fieldname, gpbmessage& val)
	{
		HANDLE_FIELD;
		if(field->is_repeated() || field->is_map())
			return false;

		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				const google::protobuf::Message& tmp = reflection->GetMessage(*message, field);
				google::protobuf::Message* newm = tmp.New();
				if(newm == NULL)
					return false;
				newm->CopyFrom(tmp);
				val.set(newm);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<typename Type>
	bool getrepeatedval(const char* fieldname, std::vector<Type>& vecVals)
	{
		HANDLE_FIELD;

		if(!field->is_repeated())
			return false;

		int count = reflection->FieldSize(*message, field);
		switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, FUNCASE)												\
		case FieldDescriptor::CPPTYPE_##UPPERCASE :									\
		for(int i = 0; i < count; ++i){												\
		Type val = reflection->GetRepeated##FUNCASE(*message, field, i);			\
		vecVals.push_back(val);														\
		}																			\
		break

			HANDLE_TYPE( INT32, Int32);
			HANDLE_TYPE( INT64, Int64);
			HANDLE_TYPE(UINT32, UInt32);
			HANDLE_TYPE(UINT64, UInt64);
			HANDLE_TYPE(DOUBLE, Double);
			HANDLE_TYPE( FLOAT, Float);
			HANDLE_TYPE(  BOOL, Bool);
			HANDLE_TYPE(  ENUM, EnumValue);
#undef HANDLE_TYPE

		default:
			return false;
		}
		return true;
	}

	template<>
	bool getrepeatedval(const char* fieldname, std::vector<std::string>& vecVals)
	{
		HANDLE_FIELD;
		if(!field->is_repeated())
			return false;

		int count = reflection->FieldSize(*message, field);
		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
			{
				for(int i = 0; i < count; ++i){
					vecVals.push_back(reflection->GetRepeatedString(*message, field, i));
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<>
	bool getrepeatedval(const char* fieldname, std::vector<gpbmessage>& vecVals)
	{
		HANDLE_FIELD;
		if(!field->is_repeated())
			return false;

		int count = reflection->FieldSize(*message, field);
		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				for(int i = 0; i < count; ++i){
					const google::protobuf::Message& tmp = reflection->GetRepeatedMessage(*message, field, i);
					google::protobuf::Message* newm = tmp.New();
					if(newm == NULL)
						return false;
					newm->CopyFrom(tmp);
					gpbmessage msg;
					msg.set(newm);
					vecVals.push_back(msg);
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<class TKey, class TVal>
	bool getmap(const char* fieldname, std::map<TKey, TVal>& mapVals)
	{
		HANDLE_FIELD;
		if(!field->is_map())
			return false;

		int count = reflection->FieldSize(*message, field);
		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				for(int i = 0; i < count; ++i){
					const google::protobuf::Message& tmp = reflection->GetRepeatedMessage(*message, field, i);
					google::protobuf::Message* newm = tmp.New();
					if(newm == NULL)
						return false;
					newm->CopyFrom(tmp);
					gpbmessage msg;
					msg.set(newm);
					TKey key;
					TVal val;
					if(msg.getval("key", key) && msg.getval("value", val))
						mapVals[key] = val;
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<typename Type>
	bool setval(const char* fieldname, Type val)
	{
		HANDLE_FIELD;
		if(field->is_repeated() || field->is_map())
			return false;

		switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, FUNCASE)									\
		case google::protobuf::FieldDescriptor::CPPTYPE_##UPPERCASE :	\
		reflection->Set##FUNCASE(message, field, val);					\
		break

			HANDLE_TYPE( INT32, Int32);
			HANDLE_TYPE( INT64, Int64);
			HANDLE_TYPE(UINT32, UInt32);
			HANDLE_TYPE(UINT64, UInt64);
			HANDLE_TYPE(DOUBLE, Double);
			HANDLE_TYPE( FLOAT, Float);
			HANDLE_TYPE(  BOOL, Bool);
			HANDLE_TYPE(  ENUM, EnumValue);
#undef HANDLE_TYPE

		default:
			return false;
		}
		return true;
	}

	template<>
	bool setval(const char* fieldname, const std::string val)
	{
		HANDLE_FIELD;
		if(field->is_repeated() || field->is_map())
			return false;

		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
			reflection->SetString(message, field, val);
			break;
		default:
			return false;
		}
		return true;
	}

	template<>
	bool setval(const char* fieldname, gpbmessage val)
	{
		HANDLE_FIELD;
		if(field->is_repeated() || field->is_map())
			return false;

		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				google::protobuf::Message* tmp = val.get();
				if(tmp == NULL){
					reflection->ClearField(message, field);
				}else{
					google::protobuf::Message* mes = reflection->MutableMessage(message, field, mGpbData->getfactory());
					if(mes == NULL)
						return false;
					mes->CopyFrom(*tmp);
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<typename Type>
	bool setrepeatedval(const char* fieldname, std::vector<Type>& vecVals)
	{
		HANDLE_FIELD;
		if(!field->is_repeated())
			return false;

		reflection->ClearField(message, field);
		int count = (int)vecVals.size();
		switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, FUNCASE)												\
		case FieldDescriptor::CPPTYPE_##UPPERCASE :									\
		for(int i = 0; i < count; ++i){												\
		reflection->Add##FUNCASE(message, field, vecVals[i]);						\
		}																			\
		break

			HANDLE_TYPE( INT32, Int32);
			HANDLE_TYPE( INT64, Int64);
			HANDLE_TYPE(UINT32, UInt32);
			HANDLE_TYPE(UINT64, UInt64);
			HANDLE_TYPE(DOUBLE, Double);
			HANDLE_TYPE( FLOAT, Float);
			HANDLE_TYPE(  BOOL, Bool);
			HANDLE_TYPE(  ENUM, EnumValue);
#undef HANDLE_TYPE

		default:
			return false;
		}
		return true;
	}

	template<>
	bool setrepeatedval(const char* fieldname, std::vector<std::string>& vecVals)
	{
		HANDLE_FIELD;
		if(!field->is_repeated())
			return false;

		reflection->ClearField(message, field);
		int count = (int)vecVals.size();
		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
			for(int i = 0; i < count; ++i){
				reflection->AddString(message, field, vecVals[i]);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<>
	bool setrepeatedval(const char* fieldname, std::vector<gpbmessage>& vecVals)
	{
		HANDLE_FIELD;
		if(!field->is_repeated())
			return false;

		reflection->ClearField(message, field);
		int count = (int)vecVals.size();
		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				for(int i = 0; i < count; ++i){
					if(vecVals[i].get()){
						google::protobuf::Message* mes = reflection->AddMessage(message, field, mGpbData->getfactory());
						if(mes == NULL)
							return false;
						mes->CopyFrom(*vecVals[i].get());
					}
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	template<class TKey, class TVal>
	bool setmap(const char* fieldname, std::map<TKey, TVal>& mapVals)
	{
		HANDLE_FIELD;
		if(!field->is_map())
			return false;

		reflection->ClearField(message, field);
		switch (field->cpp_type()) {
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				for(std::map<TKey, TVal>::iterator it = mapVals.begin(); it != mapVals.end(); ++it){
						google::protobuf::Message* mes = reflection->AddMessage(message, field, mGpbData->getfactory());
						if(mes == NULL)
							return false;
						google::protobuf::Message* newm = mes->New();
						if(newm == NULL)
							return false;
						gpbmessage msg;
						msg.set(newm);
						msg.setval("key", it->first);
						msg.setval("value", it->second);
						mes->CopyFrom(*msg.get());
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}
#undef HANDLE_FIELD


public:
	const google::protobuf::Message* get() const{return mData;}
	google::protobuf::Message* get() {return mData;}
	void set(google::protobuf::Message* pdata) {SAFE_DELETE(mData);mData=pdata;}

private:
	google::protobuf::Message* mData;
	CGpbdata* mGpbData;

private:// no new
	void* operator new(size_t sz){return ::operator new(sz);}
	void operator delete(void* p){::operator delete(p);}

public://copy
	gpbmessage(const gpbmessage& copy){
		mData = NULL;
		*this = copy;
	}
	gpbmessage &operator = (const gpbmessage& copy){
		copyfrom(copy);
		return *this;
	}
};

#endif//_GPBMESSAGE_H_

// Proto3 maps are represented on the wire as a message with
// "key" and a "value".
//
// For example, the syntax:
// map<key_type, value_type> map_field = N;
//
// is represented as:
// message MapFieldEntry {
//   option map_entry = true;   // marks the map construct in the descriptor
//
//   key_type key = 1;
//   value_type value = 2;
// }
// repeated MapFieldEntry map_field = N;
//
// See go/proto3-maps for more information.
