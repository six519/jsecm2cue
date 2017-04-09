#include <node.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include "unecm.h"

namespace ecm2cue {

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

void Process(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  char *outfilename;
  char *oldfilename;
  int unecm_ret = 0;

  FILE *fin, *fout, *cue_file;

  if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }

  String::Utf8Value param1(args[0]->ToString());
  std::string from = std::string(*param1);

  eccedc_init();

  if(strlen(from.c_str()) < 5) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "filename is too short")));
    return;
  }
  if(strcasecmp(from.c_str() + strlen(from.c_str()) - 4, ".ecm")) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "filename must end in .ecm")));
    return;
  }

  outfilename = static_cast<char*>(malloc(strlen(from.c_str()) - 3));
  oldfilename = static_cast<char*>(malloc(strlen(from.c_str()) - 3));
  if(!outfilename) abort();
  memcpy(outfilename, from.c_str(), strlen(from.c_str()) - 4);
  outfilename[strlen(from.c_str()) - 4] = 0;

  fprintf(stderr, "Decoding %s to %s.\n", from.c_str(), outfilename);

  fin = fopen(from.c_str(), "rb");
  if(!fin) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Cannot open file")));
    return;
  }
  fout = fopen(outfilename, "wb");
  if(!fout) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Cannot write file")));
    fclose(fin);
    return;
  }

  unecm_ret = unecmify(fin, fout);

  fclose(fout);
  fclose(fin);

  if(unecm_ret != 0) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Cannot decode file")));
    return;
  }

  strcpy(oldfilename, outfilename);
  cue_file = fopen(strcat(outfilename, ".cue"), "w");
  if(!cue_file) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Cannot write cue file")));
    return;
  }
  fprintf(cue_file, "FILE \"%s\" BINARY\n", oldfilename);
  fprintf(cue_file, "\tTRACK 01 MODE1/2352\n");
  fprintf(cue_file, "\t\tINDEX 01 00:00:00");
  fclose(cue_file);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, outfilename));
}

void Init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "process", Process);
}

NODE_MODULE(process, Init)

}