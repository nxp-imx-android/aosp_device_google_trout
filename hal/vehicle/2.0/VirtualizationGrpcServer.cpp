#include <android-base/logging.h>

#include "GrpcVehicleServer.h"
#include "Utils.h"

int main(int argc, char* argv[]) {
    namespace vhal_impl = android::hardware::automotive::vehicle::V2_0::impl;

    auto serverInfo = vhal_impl::VsockServerInfo::fromCommandLine(argc, argv);
    CHECK(serverInfo.has_value()) << "Invalid server CID/port combination";

    auto server = vhal_impl::makeGrpcVehicleServer(serverInfo->toUri());
    server->Start();
    return 0;
}
