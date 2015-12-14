# googleprotobufmore
google protocol buffer message dynamic analysis or create

#gpbmessage
>gpbmessage is a lib to analyze google protocol buffer message dynamically.also, you can creator a google protocol buffer message dynamically by using this lib.

**first load proto file**
```
gpbmessage::loadproto("./", "my.proto");
```
**then generate a message**
```
gpbmessage gms("mymsg");
```
**finally write message or read message**
```
gms.setval("len", 2);
std::string out;
gms.serializetostring(&out);

gpbmessage gms1("mymsg", out);
int val;
assert(gms1.getval("len", val));
```

#gpbcreator
>gpbcreator is a lib to create google protocol buffer messages and generae .proto files dynamically.

```
gpbcreator gc;
gc.newproto("my.proto");
gc.addmessage("mymsg");
gc.addfield("mymsg", "len", 1, TYPE_INT32, LABEL_OPTIONAL);
gc.save()
```
