#include <node.h>
#include <v8.h>
#include <node_buffer.h>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listCom.c"


uv_mutex_t write_queue_mutex;
ngx_queue_t write_queue;

#define ERROR_STRING_SIZE 1024

struct ListResultItem {
public:
  std::string comName;
  std::string manufacturer;
  std::string serialNumber;
  std::string pnpId;
  std::string locationId;
  std::string vendorId;
  std::string productId;
};


struct ListBaton {
public:
  v8::Persistent<v8::Value> callback;
  std::list<ListResultItem*> results;
  char errorString[ERROR_STRING_SIZE];
};
void EIO_List(uv_work_t* req) ;
void EIO_AfterList(uv_work_t* req);


v8::Handle<v8::Value> List(const v8::Arguments& args) {
  v8::HandleScope scope;

  // callback
  if(!args[0]->IsFunction()) {
    return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
  }
  v8::Local<v8::Value> callback = args[0];

  ListBaton* baton = new ListBaton();
  strcpy(baton->errorString, "");
  baton->callback = v8::Persistent<v8::Value>::New(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_List, (uv_after_work_cb)EIO_AfterList);

  return scope.Close(v8::Undefined());
}

void EIO_AfterList(uv_work_t* req) {
  ListBaton* data = static_cast<ListBaton*>(req->data);

  v8::Handle<v8::Value> argv[2];
  if(data->errorString[0]) {
    argv[0] = v8::Exception::Error(v8::String::New(data->errorString));
    argv[1] = v8::Undefined();
  } else {
    v8::Local<v8::Array> results = v8::Array::New();
    int i = 0;
    for(std::list<ListResultItem*>::iterator it = data->results.begin(); it != data->results.end(); ++it, i++) {
      v8::Local<v8::Object> item = v8::Object::New();
      item->Set(v8::String::New("comName"), v8::String::New((*it)->comName.c_str()));
      item->Set(v8::String::New("manufacturer"), v8::String::New((*it)->manufacturer.c_str()));
      item->Set(v8::String::New("serialNumber"), v8::String::New((*it)->serialNumber.c_str()));
      item->Set(v8::String::New("pnpId"), v8::String::New((*it)->pnpId.c_str()));
      item->Set(v8::String::New("locationId"), v8::String::New((*it)->locationId.c_str()));
      item->Set(v8::String::New("vendorId"), v8::String::New((*it)->vendorId.c_str()));
      item->Set(v8::String::New("productId"), v8::String::New((*it)->productId.c_str()));
      results->Set(i, item);
    }
    argv[0] = v8::Undefined();
    argv[1] = results;
  }
  v8::Function::Cast(*data->callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);

  data->callback.Dispose();
  for(std::list<ListResultItem*>::iterator it = data->results.begin(); it != data->results.end(); ++it) {
    delete *it;
  }
  delete data;
  delete req;
}

void EIO_List(uv_work_t* req) {
  
  
  ListBaton* data = static_cast<ListBaton*>(req->data);
  ListResultItem* resultItem;
  
  stDeviceListItem* devices, *temp;
  devices = NULL;
  devices = List();

  while(devices!=NULL){
      
      resultItem = new ListResultItem();
      resultItem->comName = std::string(devices->value.port);
      resultItem->vendorId = std::string(devices->value.vendorId);
      resultItem->productId = std::string(devices->value.productId);
      resultItem->manufacturer = std::string(devices->value.manufacturer);
      resultItem->serialNumber = std::string(devices->value.serialNumber);
      resultItem->locationId = std::string(devices->value.locationId);
      temp = devices;
      devices = devices->next;
      free(temp);
      
      data->results.push_back(resultItem);
  }

    
  
      
}

void init(v8::Handle<v8::Object> exports) {
    exports->Set(v8::String::NewSymbol("list"),
                 v8::FunctionTemplate::New(List)->GetFunction());
}

NODE_MODULE(listcom, init)




