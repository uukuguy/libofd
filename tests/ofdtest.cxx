#include <iostream>
#include "OFDFile.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "logger.h"

using namespace ofd;

int main(int argc, char *argv[]){
    if ( argc <= 1 ){
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 0;
    }

    TIMED_FUNC(timerMain);

    Logger::Initialize(argc, argv);

    LOG(INFO) << "Start " << argv[0];

    OFDFile ofdFile;
    if ( ofdFile.Open(argv[1]) ){
        OFDDocument *ofdDocument = ofdFile.GetOFDDocument(); 
        LOG(DEBUG) << ofdDocument->String();

        size_t n_pages = ofdDocument->GetPagesCount();
        LOG(INFO) << "Loading " << n_pages << " pages.";

        for ( size_t i = 0 ; i < n_pages ; i++ ){

            OFDPage *ofdPage = ofdDocument->GetOFDPage(i);
            ofdPage->Open();

            VLOG(3) << ofdPage->String(); 

            LOG(INFO) << ofdPage->GetText();

            ofdPage->Close();
        }

        ofdFile.Close();
    }

    LOG(INFO) << "Done.";

    return 0;
}
