#ifndef _GPBCREATOR_H_
#define _GPBCREATOR_H_
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <string>
#include <map>
#include <iostream>

enum Type {
	TYPE_DOUBLE         = 1,   // double, exactly eight bytes on the wire.
	TYPE_FLOAT          = 2,   // float, exactly four bytes on the wire.
	TYPE_INT64          = 3,   // int64, varint on the wire.  Negative numbers
	// take 10 bytes.  Use TYPE_SINT64 if negative
	// values are likely.
	TYPE_UINT64         = 4,   // uint64, varint on the wire.
	TYPE_INT32          = 5,   // int32, varint on the wire.  Negative numbers
	// take 10 bytes.  Use TYPE_SINT32 if negative
	// values are likely.
	TYPE_FIXED64        = 6,   // uint64, exactly eight bytes on the wire.
	TYPE_FIXED32        = 7,   // uint32, exactly four bytes on the wire.
	TYPE_BOOL           = 8,   // bool, varint on the wire.
	TYPE_STRING         = 9,   // UTF-8 text.
	TYPE_GROUP          = 10,  // Tag-delimited message.  Deprecated.
	TYPE_MESSAGE        = 11,  // Length-delimited message.

	TYPE_BYTES          = 12,  // Arbitrary byte array.
	TYPE_UINT32         = 13,  // uint32, varint on the wire
	TYPE_ENUM           = 14,  // Enum, varint on the wire
	TYPE_SFIXED32       = 15,  // int32, exactly four bytes on the wire
	TYPE_SFIXED64       = 16,  // int64, exactly eight bytes on the wire
	TYPE_SINT32         = 17,  // int32, ZigZag-encoded varint on the wire
	TYPE_SINT64         = 18,  // int64, ZigZag-encoded varint on the wire

	MAX_TYPE            = 18,  // Constant useful for defining lookup tables
	// indexed by Type.
};

enum Label {
	LABEL_OPTIONAL      = 1,    // optional
	LABEL_REQUIRED      = 2,    // required
	LABEL_REPEATED      = 3,    // repeated

	MAX_LABEL           = 3,    // Constant useful for defining lookup tables
	// indexed by Label.
};

class gpbctrerror : public google::protobuf::DescriptorPool::ErrorCollector
{
public:
	gpbctrerror(){}
	~gpbctrerror(){}

	virtual void AddError(
		const std::string& filename,      // File name in which the error occurred.
		const std::string& element_name,  // Full name of the erroneous element.
		const google::protobuf::Message* descriptor,   // Descriptor of the erroneous element.
		google::protobuf::DescriptorPool::ErrorCollector::ErrorLocation location,      // One of the location constants, above.
		const std::string& message        // Human-readable error message.
		){
			std::cerr<<"error: file="<<filename<<" element_name "<<element_name<<std::endl<<"err="<<message<<std::endl;
	}
};

class gpbcreator
{
public:
	gpbcreator();
	~gpbcreator();

public:
	bool newproto(const char* name, const char* version="proto3");
	bool addmessage(const char* name);
	bool addfield(const char* messagename, const char* fieldname, int number, int itype, int label=LABEL_OPTIONAL, const char* fmname=NULL);
	bool addmapfield(const char* messagename, const char* fieldname, int number, int keytype, int valtype, const char* fmname=NULL);
	bool save(const char* path="./");

private:
	void destory();
	google::protobuf::DescriptorProto* getmessage(const char* messagename);

private:
	google::protobuf::FileDescriptorProto* mFileproto;
	std::map<std::string, google::protobuf::DescriptorProto*> mMessagesMap;
	std::string mFileName;
	std::string mSyntax;

	gpbctrerror mError;
};

#endif//_GPBCREATOR_H_

