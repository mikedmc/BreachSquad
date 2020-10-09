//all version numbers must be between 0 and 99
#define _VERSION_MAJOR_ 1
#define _VERSION_MINOR_ 2
#define _VERSION_PATCH_ 8

#define _VERSION_CHARSTR_ TOSTRING(_VERSION_MAJOR_._VERSION_MINOR_._VERSION_PATCH_)
#define _VERSION_INT_ (_VERSION_MAJOR_ * 10000 + _VERSION_MINOR_ * 100 + _VERSION_PATCH_)
#define _VERSION_BYTE_ (_VERSION_MINOR_ * 100 + _VERSION_PATCH_)
#define _VERSION_HALFBYTE_ (_VERSION_MINOR_ + _VERSION_PATCH_)


// Minimum version of compatible mods 
// Change this to current version when we have mod breaking updates
#define _VERSION_MINMOD_MAJOR_ 1
#define _VERSION_MINMOD_MINOR_ 2
#define _VERSION_MINMOD_PATCH_ 2
#define _VERSION_MINMOD_WCHARSTR_ L"1.2.2"


//data file version - separate version for the file where user data gets saved. Increment when file format changes!
#define _VERSION_DATAFILE_	517
//v514 to 515 - updated g_levelStats (added more items)
//v515 to 516 - added CShop class that replaced the old f_weaponShop
//v516 to 517 - final launch, no changes, forcing user reset


