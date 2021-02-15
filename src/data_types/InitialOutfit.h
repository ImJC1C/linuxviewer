#pragma once

#include <string>
#include "Gender.h"
#include "protocols/xmlrpc/initialize.h"
#include "evio/protocol/xmlrpc/macros.h"

#define xmlrpc_InitialOutfit_FOREACH_MEMBER(X) \
  X(std::string, folder_name) \
  X(Gender, gender)

class InitialOutfit
{
 private:
  xmlrpc_InitialOutfit_FOREACH_MEMBER(XMLRPC_DECLARE_MEMBER)

 public:
  enum members {
    xmlrpc_InitialOutfit_FOREACH_MEMBER(XMLRPC_DECLARE_ENUMERATOR)
  };

  xmlrpc::ElementDecoder* create_member_decoder(members member);
};
