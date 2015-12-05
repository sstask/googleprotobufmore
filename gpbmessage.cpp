#include "gpbmessage.h"

////////////////////////////CGpbdata
CGpbdata* CGpbdata::mInstance = NULL;

CGpbdata::CGpbdata():mMessageCount(0)
{

}

CGpbdata::~CGpbdata()
{

}

void CGpbdata::destory()
{
    if(mMessageCount > 0)
        return;
	SAFE_DELETE(mFactory);
	SAFE_DELETE(mImporter);
	mMessageFieldMap.clear();
	mMessageMap.clear();
}

bool CGpbdata::loadproto(const char* path, const char* file)
{
	if(mMessageCount > 0)
		return false;
	destory();
	google::protobuf::compiler::DiskSourceTree sourceTree;
	sourceTree.MapPath("", path);
	mImporter = new google::protobuf::compiler::Importer(&sourceTree, &mError);
	if(mImporter == NULL)
		return false;
	const google::protobuf::FileDescriptor* pfd = mImporter->Import(file);
	if (pfd == NULL)
	{
		SAFE_DELETE(mImporter);
		return false;
	}
	mFactory = new google::protobuf::DynamicMessageFactory();
	if (mFactory == NULL)
	{
		destory();
		return false;
	}
	return true;
}

bool CGpbdata::newmessage(gpbmessage& msg, const char* msgname)
{
	const google::protobuf::Message* message = getmessage(msgname);
	if(message){
		google::protobuf::Message* tmp = message->New();
		if(tmp){
			msg.set(tmp);
			return true;
		}
	}
	return false;
}

bool CGpbdata::getmessagefromstring(gpbmessage& msg, const char* msgname, const std::string& content)
{
	if(newmessage(msg, msgname)){
		msg.get()->ParseFromString(content);
		return true;
	}
	return false;
}

const google::protobuf::Message* CGpbdata::getmessage(const char* msgname)
{
	if(mImporter == NULL || msgname == NULL)
		return NULL;

	GPBMESSAGEMAP::iterator itr = mMessageMap.find(msgname);
	if (itr != mMessageMap.end()){
		return itr->second;
	}
	const google::protobuf::Descriptor *descriptor = mImporter->pool()->FindMessageTypeByName(msgname);
	if(descriptor){
		const google::protobuf::Message *message = mFactory->GetPrototype(descriptor);
		mMessageMap[msgname] = message;

		for(int i = 0; i < descriptor->field_count(); ++i){
			const google::protobuf::FieldDescriptor* pfd = descriptor->field(i);
			mMessageFieldMap[std::string(msgname)+"+"+pfd->name()] = pfd;
		}
		return message;
	}

	mMessageMap[msgname] = NULL;
	return NULL;
}

////////////////////////////gpbmessage
gpbmessage::gpbmessage():mData(NULL),mGpbData(NULL)
{
	mGpbData = CGpbdata::getinstance();
	if(mGpbData)
		mGpbData->add();
}

gpbmessage::gpbmessage(const char* mname):mData(NULL),mGpbData(NULL)
{
	mGpbData = CGpbdata::getinstance();
	if(mGpbData && mname){
		mGpbData->newmessage(*this, mname);
	}
	if(mGpbData)
		mGpbData->add();
}

gpbmessage::gpbmessage(const char* mname, const std::string& content):mData(NULL),mGpbData(NULL)
{
	mGpbData = CGpbdata::getinstance();
	if(mGpbData && mname){
		if(!content.empty())
			mGpbData->getmessagefromstring(*this, mname, content);
		else
			mGpbData->newmessage(*this, mname);
	}
	if(mGpbData)
		mGpbData->add();
}

gpbmessage::~gpbmessage()
{
	SAFE_DELETE(mData);
	if(mGpbData)
		mGpbData->sub();
}

bool gpbmessage::loadproto(const char* path, const char* file)
{
	CGpbdata* pgpb = CGpbdata::getinstance();
	if(pgpb)
		return pgpb->loadproto(path, file);
	return false;
}

void gpbmessage::destoryproto()
{
	CGpbdata::delinstance();
}

bool gpbmessage::copyfrom(const gpbmessage& copy)
{
	SAFE_DELETE(mData);
	mGpbData = CGpbdata::getinstance();
	const google::protobuf::Message* mess = copy.get();
	if(mess == NULL)
		return true;
	mData = mess->New();
	if(mData == NULL)
		return false;
	mData->CopyFrom(*mess);
	if(mGpbData)
		mGpbData->add();
	return true;
}

bool gpbmessage::serializetostring(std::string* output)
{
	if(mData){
		return mData->SerializeToString(output);
	}
	return true;
}

bool gpbmessage::parsefromstring(const char* mname, const char* content)
{
	SAFE_DELETE(mData);
	if(mGpbData)
		return mGpbData->getmessagefromstring(*this, mname, content);
	return false;
}

std::string gpbmessage::shortdebugstring()
{
	if(mData){
		return mData->ShortDebugString();
	}
	return std::string("");
}
