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
#include "logger.h"

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

DEFINE_string(pdffile, "", "The PDF file to translated.");

int main(int argc, char *argv[]){

    Logger::Initialize(argc, argv);

    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Init poppler.
    globalParams = new GlobalParams(nullptr);

    std::string pdfFilename = argv[1];
    std::string ownerPassword = "";
    std::string userPassword = "";
    LOG(DEBUG) << "Try to open pdf file: " << pdfFilename;

    PDFDoc *pdfDoc = OpenPDFFile(pdfFilename, ownerPassword, userPassword);
    if ( pdfDoc != nullptr ){

        OFDOutputDev *ofdOut = new OFDOutputDev();
        ofdOut->ProcessDoc(pdfDoc);

        delete ofdOut;
        ofdOut = nullptr;

        delete pdfDoc;
        pdfDoc = nullptr;
    }

    delete globalParams;

    return 0;
}
