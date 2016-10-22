#include <iostream>
#include "OFDPackage.h"
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

    OFDPackage ofdPackage;
    if ( ofdPackage.Open(argv[1]) ){
        OFDDocument *ofdDocument = ofdPackage.GetOFDDocument(); 
        LOG(DEBUG) << ofdDocument->String();

        size_t n_pages = ofdDocument->GetPagesCount();
        LOG(INFO) << "Loading " << n_pages << " pages.";

        for ( size_t i = 0 ; i < n_pages ; i++ ){

            OFDPage *ofdPage = ofdDocument->GetOFDPage(i);
            ofdPage->Open();

            VLOG(3) << ofdPage->String(); 
            LOG(INFO) << ofdPage->GetText();

            std::stringstream ss;
            ss << "Page" << (i + 1) << ".png";
            std::string png_filename = ss.str();
            ofdPage->RenderToPNGFile(png_filename);

            ofdPage->Close();
        }

        ofdPackage.Close();
    }

    LOG(INFO) << "Done.";

    return 0;
}
