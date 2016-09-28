#ifndef _antlr_interface_h_
#define _antlr_interface_h_

#include <memory>
#include <string>
#include <jni.h>

class antlr {
 public:
  antlr();
  bool parse(const std::string& expr, std::string& result) const;
  void async_parse(const std::string expr) const;

 private:
  jclass CalcCls;
  jmethodID parseId;
  jclass StringCls;
};
#endif
