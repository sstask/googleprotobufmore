#include "gpbcreator.h"
#include <fstream>

gpbcreator::gpbcreator()
{
}


gpbcreator::~gpbcreator()
{
	destory();
}

void gpbcreator::destory()
{
	if(mFileproto){
		delete mFileproto;
		mFileproto = NULL;
	}
}

bool gpbcreator::newproto(const char* name, const char* version)
{
	destory();
	mFileproto = new google::protobuf::FileDescriptorProto();
	if(mFileproto == NULL)
		return false;
	mFileproto->set_name(name);
	mFileproto->set_syntax(version);
	mFileName = name;
	mSyntax = version;
	return true;
}

bool gpbcreator::addmessage(const char* name)
{
	if(mFileproto == NULL)
		return false;
	std::map<std::string, google::protobuf::DescriptorProto*>::iterator it = mMessagesMap.find(name);
	if(it != mMessagesMap.end()){
		return true;
	}

	google::protobuf::DescriptorProto* messageproto = mFileproto->add_message_type();
	if(messageproto == NULL)
		return false;
	messageproto->set_name(name);
	mMessagesMap[name] = messageproto;
	return true;
}

google::protobuf::DescriptorProto* gpbcreator::getmessage(const char* messagename)
{
	if(mFileproto == NULL)
		return false;
	std::map<std::string, google::protobuf::DescriptorProto*>::iterator it = mMessagesMap.find(messagename);
	if(it == mMessagesMap.end()){
		addmessage(messagename);
	}
	return mMessagesMap[messagename];
}

bool gpbcreator::addfield(const char* messagename, const char* fieldname, int number, int itype, int label, const char* fmname)
{
	if(mFileproto == NULL)
		return false;
	if(itype == TYPE_MESSAGE && fmname == NULL)
	{
		std::cerr<<"error: msg["<<messagename<<"] filed["<<fieldname<<"], type is message but para [fmname]=null"<<std::endl;
		return false;
	}
	google::protobuf::DescriptorProto* messageproto = getmessage(messagename);
	if(messageproto == NULL)
		return false;

	google::protobuf::FieldDescriptorProto* fieldproto = messageproto->add_field();
	if(fieldproto == NULL)
		return false;

	fieldproto->set_name(fieldname);
	fieldproto->set_number(number);
	fieldproto->set_type((google::protobuf::FieldDescriptorProto_Type)itype);
	fieldproto->set_label((google::protobuf::FieldDescriptorProto_Label)label);

	if (itype == TYPE_MESSAGE)
		fieldproto->set_type_name(fmname);
	return true;
}

std::string MapEntryName(const std::string& field_name) {
	std::string result;
	static const char kSuffix[] = "Entry";
	result.reserve(field_name.size() + sizeof(kSuffix));
	bool cap_next = true;
	for (int i = 0; i < field_name.size(); ++i) {
		if (field_name[i] == '_') {
			cap_next = true;
		} else if (cap_next) {
			// Note: Do not use ctype.h due to locales.
			if ('a' <= field_name[i] && field_name[i] <= 'z') {
				result.push_back(field_name[i] - 'a' + 'A');
			} else {
				result.push_back(field_name[i]);
			}
			cap_next = false;
		} else {
			result.push_back(field_name[i]);
		}
	}
	result.append(kSuffix);
	return result;
}

bool gpbcreator::addmapfield(const char* messagename, const char* fieldname, int number, int keytype, int valtype, const char* fmname)
{
	if(mFileproto == NULL)
		return false;
	if(valtype == TYPE_MESSAGE && fmname == NULL){
		std::cerr<<"error: msg["<<messagename<<"] filed["<<fieldname<<"], type is message but para [fmname]=null"<<std::endl;
		return false;
	}
	google::protobuf::DescriptorProto* messageproto = getmessage(messagename);
	if(messageproto == NULL)
		return false;
	google::protobuf::FieldDescriptorProto* field = messageproto->add_field();
	if(field == NULL)
		return false;
	field->set_name(fieldname);
	field->set_number(number);
	field->set_type((google::protobuf::FieldDescriptorProto_Type)TYPE_MESSAGE);
	field->set_label((google::protobuf::FieldDescriptorProto_Label)LABEL_REPEATED);

	google::protobuf::RepeatedPtrField<google::protobuf::DescriptorProto>* messages = messageproto->mutable_nested_type();
	google::protobuf::DescriptorProto* entry = messages->Add();
	std::string entry_name = MapEntryName(field->name());
	field->set_type_name(entry_name);
	entry->set_name(entry_name);
	entry->mutable_options()->set_map_entry(true);
	google::protobuf::FieldDescriptorProto* key_field = entry->add_field();
	key_field->set_name("key");
	key_field->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
	key_field->set_number(1);
	key_field->set_type((google::protobuf::FieldDescriptorProto_Type)keytype);
	google::protobuf::FieldDescriptorProto* value_field = entry->add_field();
	value_field->set_name("value");
	value_field->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
	value_field->set_number(2);
	value_field->set_type((google::protobuf::FieldDescriptorProto_Type)valtype);
	if (valtype == TYPE_MESSAGE)
		value_field->set_type_name(fmname);
	return true;
}

bool gpbcreator::save(const char* path)
{
	if(mFileproto == NULL)
		return false;

	google::protobuf::DescriptorPool pool;
	const google::protobuf::FileDescriptor *filedescriptor = pool.BuildFileCollectingErrors(*mFileproto, &mError);
	if(filedescriptor == NULL)
		return false;

	std::ofstream file(std::string(path)+mFileName);
	if(!file.is_open())
		return false;
	if(mSyntax == "proto3"){
		std::string content = filedescriptor->DebugString();
		std::string::size_type pos(0);
		while((pos=content.find("optional", pos+1))!=std::string::npos){
#define ISAN(X) ((X>='a'&&X<='z')||(X>='A'&&X<='Z')||(X>='0'&&X<='9')||X=='_')
			char a = content[pos-1];
			char b = content[pos+8];
			if(!ISAN(a) && !ISAN(b)){
				content.replace(pos,9,"");
			}
#undef ISAN
		}
		file << content;
	}else{
		file << filedescriptor->DebugString();
	}
	file.close();
	return true;
}
