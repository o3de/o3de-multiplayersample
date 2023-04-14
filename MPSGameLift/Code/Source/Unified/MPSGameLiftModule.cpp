

#include <MPSGameLiftModuleInterface.h>
#include "MPSGameLiftSystemComponent.h"

namespace MPSGameLift
{
    class MPSGameLiftModule
        : public MPSGameLiftModuleInterface
    {
    public:
        AZ_RTTI(MPSGameLiftModule, "{83F11C38-6C62-49AC-B9AA-3AF337783A80}", MPSGameLiftModuleInterface);
        AZ_CLASS_ALLOCATOR(MPSGameLiftModule, AZ::SystemAllocator, 0);
    };
}// namespace MPSGameLift

#if defined(AZ_MONOLITHIC_BUILD)
AZ_DECLARE_MODULE_CLASS(Gem_MPSGameLift_Client, MPSGameLift::MPSGameLiftModule);
AZ_DECLARE_MODULE_CLASS(Gem_MPSGameLift_Server, MPSGameLift::MPSGameLiftModule);
#endif
AZ_DECLARE_MODULE_CLASS(Gem_MPSGameLift, MPSGameLift::MPSGameLiftModule)
