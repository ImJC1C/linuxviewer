#include "sys.h"
#include "InventoryFolder.h"
#include "evio/protocol/xmlrpc/create_member_decoder.h"

xmlrpc::ElementDecoder* InventoryFolder::create_member_decoder(members member)
{
  switch (member)
  {
    xmlrpc_InventoryFolder_FOREACH_MEMBER(XMLRPC_CASE_RETURN_MEMBER_DECODER)
  }
}
