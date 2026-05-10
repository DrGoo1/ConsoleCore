#include "RoutingMatrix.h"

namespace consolecore
{
void RoutingMatrix::clear()
{
    routes.clear();
}

void RoutingMatrix::addRoute(int inputChannel, int outputChannel, float gain)
{
    routes.push_back({ inputChannel, outputChannel, gain });
}
}
