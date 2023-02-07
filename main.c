#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_STRING_SIZE 100

/*FUNTION DECLARATIONS*/

// funtion to check if the newConfigFile passed as parameter exists
int newConfigFileExists(const char *newConfigFilename);

// function that checks a features log newConfigFile to know if a test passed
int testPassed(char *path);

// function which adds the features required for the vm or application to start to the featuresRequired newConfigFile
void addFeatureToRequiredFeatureList(char *feature);

/* MAIN FUNCTION
Our main code which iterates through the list of features
and creates a directory which contains the log of the make check command for features
and a file which contains the features which either the vm or application does not start
*/
int main()
{
    // LIST OF FEATURES
    char features[][132] = {"3dnow", "3dnowext", "3dnowprefetch", "abm", "acpi", "adx", "aes", "altmovcr8", "apic", "arat", "avx", "avx2", "avx512-4fmaps", "avx512-4vnniw", "avx512bw", "avx512cd", "avx512dq", "avx512er", "avx512f", "avx512ifma", "avx512pf", "avx512vbmi", "avx512vl", "bmi1", "bmi2", "clflushopt", "clfsh", "clwb", "cmov", "cmplegacy", "cmpxchg16", "cmpxchg8", "cmt", "cntxid", "dca", "de", "ds", "dscpl", "dtes64", "erms", "est", "extapic", "f16c", "ffxsr", "fma", "fma4", "fpu", "fsgsbase", "fxsr", "hle", "htt", "hypervisor", "ia64", "ibs", "invpcid", "invtsc", "lahfsahf", "lm", "lwp", "mca", "mce", "misalignsse", "mmx", "mmxext", "monitor", "movbe", "mpx", "msr", "mtrr", "nodeid", "nx", "ospke", "osvw", "osxsave", "pae", "page1gb", "pat", "pbe", "pcid", "pclmulqdq", "pdcm", "perfctr_core", "perfctr_nb", "pge", "pku", "popcnt", "pse", "pse36", "psn", "rdrand", "rdseed", "rdtscp", "rtm", "sha", "skinit", "smap", "smep", "smx", "ss", "sse", "sse2", "sse3", "sse4.1", "sse4.2", "sse4_1", "sse4_2", "sse4a", "ssse3", "svm", "svm_decode", "svm_lbrv", "svm_npt", "svm_nrips", "svm_pausefilt", "svm_tscrate", "svm_vmcbclean", "syscall", "sysenter", "tbm",
                            "tm", "tm2", "topoext", "tsc", "tsc-deadline", "tsc_adjust", "umip", "vme", "vmx", "wdt", "x2apic", "xop", "xsave", "xtpr"};

    // FILE DECLARATIONS
    FILE *newConfigFile;
    FILE *sourceFile;
    FILE *logFile;
    FILE *featureFile;
    FILE *featuresRequired;

    // STRING INITIALISATIONS
    char nameOfVm[] = "vmjoe";
    char pathToOriginalConfigFile[] = "./vmjoe.cfg";
    char pathToNewConfigFile[] = "./vmImageConfig.cfg";
    char pathToMakeCheckLogFile[] = "./logfile.txt";
    char pathToFeaturesTestedDirectory[] = "./featuresTested/";

    // Iterating over the list of features
    for (int i = 0; i < sizeof(features) / sizeof(features[0]); i++)
    {

        printf("Testing feature: %s at index %d \n", features[i], i);

        // Copying the default configurations from the vmjoe.cfg file to a new config file the new_vmjoe_instance.cfg

        sourceFile = fopen(pathToOriginalConfigFile, "r");
        newConfigFile = fopen(pathToNewConfigFile, "w");
        char ch = fgetc(sourceFile);
        while (ch != EOF)
        {
            fputc(ch, newConfigFile);

            ch = fgetc(sourceFile);
        }

        fclose(newConfigFile);
        fclose(sourceFile);

        // Opening the new config file now in append mode to add the feature to the cpuid

        newConfigFile = fopen(pathToNewConfigFile, "a");
        char cpuidStr[MAX_STRING_SIZE] = "cpuid=\"host,";
        strcat(cpuidStr, features[i]);
        strcat(cpuidStr, "=0\"");
        fprintf(newConfigFile, "%s", cpuidStr);

        fclose(newConfigFile);

        // Executing the system call to create the vm with the new configuration file

        char createVm[MAX_STRING_SIZE] = "xl create ";
        strcat(createVm, pathToNewConfigFile);
        int vmCreated = system(createVm);

        // Add the feature to the list of required features if it required to start the VM

        if (vmCreated != 0){
            printf("The vm did not start with the feature: %s at index %d", features[i], i);

            addFeatureToRequiredFeatureList(features[i]);
            continue;
        }else{
            printf("The vm started with the feature: %s at index %d", features[i], i);

            /*
            REMEMBER: We have a script teshScript.sh which is run on startup of our VM.
            we created a startup.service which is references this script such that everytime
            the VM starts the service is called and it runs the script.

            This script runs postgresql on the vm, runs make check and returns its logfile to the dom 0

            */

            // Create a directory called featuresTested which will contain the logFiles for all our tested features
            system("mkdir -p featuresTested");

            // Wait untill the make check logfile is available before continuing the code
            while (newConfigFileExists(pathToMakeCheckLogFile) != 1)
                continue;

            // Can now shutdown the vm
            char shutDownVm[MAX_STRING_SIZE] = "xl shutdown ";
            strcat(shutDownVm, nameOfVm);
            system(shutDownVm);

            // Create a new file with the feature name in a directory which contains the logs of all the features
            logFile = fopen(pathToMakeCheckLogFile, "r");
            char path[MAX_STRING_SIZE] = "./featuresTested/";
            strcat(path, features[i]);
            strcat(path, ".txt");

            featureFile = fopen(path, "w");

            ch = fgetc(logFile);
            while (ch != EOF)
            {
                fputc(ch, featureFile);

                ch = fgetc(logFile);
            }

            fclose(featureFile);
            fclose(logFile);

            // Delete the logfile sent from the vm for this iteration
            char removeLogFile[MAX_STRING_SIZE] = "rm ";
            strcat(removeLogFile, pathToMakeCheckLogFile);
            system(removeLogFile);

            printf("Test passed: %d\n", testPassed(path));

            if (testPassed(path) == 0)
            {
                addFeatureToRequiredFeatureList(features[i]);
            }

            // hault for sometime while the vm shutsdown
            sleep(13);
        }
    }

    printf("Iterated through all features");

    return 0;
}

/*FUNTION INITIALISATIONS*/

int newConfigFileExists(const char *newConfigFilename)
{
    FILE *fp = fopen(newConfigFilename, "r");
    int is_exist = 0;
    if (fp != NULL)
    {
        is_exist = 1;
        fclose(fp);
    }
    return is_exist;
}

int testPassed(char *path)
{
    FILE *fp;
    int success = 0;
    char *successMessage = "All 209 tests passed.";

    fp = fopen(path, "r");
    char buf[100];
    int myNumber = -1;
    while ((fgets(buf, 100, fp) != NULL))
    {
        if (strstr(buf, successMessage) != NULL)
        {
            success = 1;
            break;
        }
    }
    return success;
    fclose(fp);
}

void addFeatureToRequiredFeatureList(char *feature)
{

    FILE *featuresRequired;

    featuresRequired = fopen("./featuresRequired.txt", "a");

    fprintf(featuresRequired, "%s", feature);

    fprintf(featuresRequired, "\n");

    printf("%s feature was added to list ", feature);

    fclose(featuresRequired);
}