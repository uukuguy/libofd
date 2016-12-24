#include <iostream>
#include <iomanip>
#include <gflags/gflags.h>

// -------- poppler --------
#include <poppler-config.h>
#include <goo/GooString.h>

#include <Object.h>
#include <PDFDoc.h>
#include <PDFDocFactory.h>
#include <GlobalParams.h>

#include "OFDOutputDev.h"
#include "utils/logger.h"

PDFDoc* OpenPDFFile(const std::string &pdfFilename, const std::string &ownerPassword, const std::string &userPassword){
    GooString fileName(pdfFilename.c_str());
    GooString *ownerPW = (ownerPassword == "") ? nullptr: new GooString(ownerPassword.c_str()); 
    GooString *userPW = (userPassword == "") ? nullptr : new GooString(userPassword.c_str());

    PDFDoc *pdfDoc = PDFDocFactory().createPDFDoc(fileName, ownerPW, userPW);
    if ( pdfDoc != nullptr ){
        bool ok = false;
        if ( pdfDoc->isOk() ){
            if (pdfDoc->okToCopy()){
                ok = true;
            } else {
                LOG(ERROR) << "PDF file is not okToCopy. " << pdfFilename;
            }
        } else {
            LOG(ERROR) << "Cann't read pdf file: " << pdfFilename;
        }

        if ( !ok ){
            delete pdfDoc;
            pdfDoc = nullptr;
        }
    } else {
        LOG(WARNING) << "pdfDoc == nullptr";
    }

    if ( ownerPW != nullptr ){
        delete ownerPW;
        ownerPW = nullptr;
    }
    if ( userPW != nullptr ){
        delete userPW;
        userPW = nullptr;
    }

    return pdfDoc;
}

DEFINE_int32(v, 0, "Logger level.");
//DEFINE_string(pdffile, "", "The PDF file to translated.");
DEFINE_string(owner_password, "", "The owner password of PDF file.");
DEFINE_string(user_password, "", "The user password of PDF file.");

int main(int argc, char *argv[]){

    TIMED_FUNC(timerMain);

    gflags::SetVersionString("1.0.0");
    gflags::SetUsageMessage("Usage: ./pdf2ofd <pdffile> [ofdfile]");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Logger::Initialize(FLAGS_v);

    // pdf filename
    if ( argc < 2 ){
        LOG(WARNING) << "Usage: ./pdf2ofd <pdffile> [ofdfile]";
        exit(-1);
    }
    std::string pdfFilename = argv[1];

    // ofd filename
    std::string ofdPackagename = pdfFilename + ".ofd";
    if ( argc > 2 ) {
        ofdPackagename = argv[2];
    }
    LOG(INFO) << "Try to translate pdf file " << pdfFilename << " to ofd file " << ofdPackagename;

    // owner password
    std::string ownerPassword = FLAGS_owner_password;
    // user password
    std::string userPassword = FLAGS_user_password;

    // Init poppler.
    globalParams = new GlobalParams(nullptr);


    PDFDoc *pdfDoc = OpenPDFFile(pdfFilename, ownerPassword, userPassword);
    if ( pdfDoc != nullptr ){


        ofd::OFDPackagePtr ofdPackage = std::make_shared<ofd::OFDPackage>();

        LOG(DEBUG) << "------- 0 ========";
        OFDOutputDev *ofdOut = new OFDOutputDev(ofdPackage);
        //std::shared_ptr<OFDOutputDev> ofdOut = std::make_shared<OFDOutputDev>(ofdPackage);

        LOG(DEBUG) << "------- 1 ========";
        ofdOut->ProcessDoc(pdfDoc);

        LOG(DEBUG) << "------- 5 ========";
        // Fonts
        const std::map<int, std::shared_ptr<ofd::OFDFont> > &fonts = ofdOut->GetFonts();
        ofd::OFDDocumentPtr ofdDoc = ofdPackage->GetDocument(0);
        __attribute__((unused)) ofd::OFDDocument::CommonData &commonData = ofdDoc->GetCommonData();
        LOG(INFO) << "--------- fonts.size()=" << fonts.size();
        for ( auto font :fonts){
            if ( font.second != nullptr ){
                if (commonData.PublicRes != nullptr ){
                    LOG(DEBUG) << "Font Name: " << font.second->FontName;
                    commonData.PublicRes->AddFont(*(font.second));
                } else {
                    LOG(ERROR) << "commonData.PublicRes == nullptr";
                }
            } else {
                LOG(WARNING) << "font in fonts is nullptr";
            }
        }

        LOG(DEBUG) << "------- 9 ========";
        ofdPackage->Save(ofdPackagename);

        delete ofdOut;
        ofdOut = nullptr;

        delete pdfDoc;
        pdfDoc = nullptr;
    }

    delete globalParams;

    return 0;
}
