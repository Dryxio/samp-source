// Direct vendor factory Router creation and destruction.
#include "../raknet/RakNetworkFactory.h"
#include "../raknet/Router.h"
Router* RakNetworkFactory::GetRouter(void) { return new Router; }
void RakNetworkFactory::DestroyRouter(Router* i) { delete (Router*)i; }
typedef char R5RouterSize[(sizeof(Router)==29)?1:-1];
