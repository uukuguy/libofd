#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cairo/cairo.h>
#include <assert.h>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "logger.h"

using namespace ofd;

// -------- create_image_surface() --------
SDL_Surface *create_image_surface(int width, int height, int bpp){
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000; 
    gmask = 0x00ff0000; 
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000; 
    amask = 0xff000000; 
#endif

    SDL_Surface *imageSurface = SDL_CreateRGBSurface(0, width, height, bpp, 
            rmask, gmask, bmask, amask);
    if ( imageSurface == nullptr ){
        LOG(ERROR) << "SDL_CreateRGBSurface() failed. " << SDL_GetError();
    }

    return imageSurface;
}

// -------- create_cairo_surface_from_sdl_surface() --------
cairo_surface_t *create_cairo_surface_from_sdl_surface(SDL_Surface *sdl_surface){
    cairo_surface_t *cairo_surface = nullptr;

    cairo_surface = cairo_image_surface_create_for_data(
            (unsigned char*)sdl_surface->pixels,
            CAIRO_FORMAT_ARGB32,
            sdl_surface->w,
            sdl_surface->h,
            sdl_surface->pitch
            );

    return cairo_surface;
}

// **************** class SDLApp ****************
class SDLApp {
public:
    SDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP);
    virtual ~SDLApp();

    void Execute();

protected:
    virtual void OnEvent(SDL_Event event, bool &done);
    virtual void OnRender(cairo_surface_t *surface);

    void RenderScreen() const;
    void RefreshScreen(uint64_t color) const;
    void ToggleFullScreen();

private:
    std::string m_title;
    bool m_fullscreen;
    double m_screenWidth;
    double m_screenHeight;
    int m_screenBPP;

    SDL_Window *m_mainWindow;
    SDL_Renderer *m_screenRenderer;

    bool init();
    void cleanup();
    void Loop();

protected:
    SDL_Surface *m_imageSurface;

}; // class SDLApp

// ======== SDLApp::SDLApp() ========
SDLApp::SDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP)
    : m_title(title), m_fullscreen(false), 
    m_screenWidth(screenWidth), m_screenHeight(screenHeight), m_screenBPP(screenBPP),
    m_mainWindow(nullptr), m_screenRenderer(nullptr), m_imageSurface(nullptr){
    init();
}

// ======== SDLApp::~SDLApp() ========
SDLApp::~SDLApp(){
    cleanup();
}


bool SDLApp::init(){
    int screenFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE; // SDL_WINDOW_BORDERLESS
    m_mainWindow = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_screenWidth, m_screenHeight, screenFlags);
    if ( m_mainWindow == nullptr ){
        LOG(ERROR) << "SDL_CreateWindow() failed. " << SDL_GetError();
        return false;
    }

    m_screenRenderer = SDL_CreateRenderer(m_mainWindow, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
    if ( m_screenRenderer == nullptr ){
        LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
        return false;
    }

    if ( !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ){
        LOG(ERROR) << "IMG_Init() failed. " << SDL_GetError();
        return false;
    }

    m_imageSurface = create_image_surface(m_screenWidth, m_screenHeight, m_screenBPP);
    if ( m_imageSurface == nullptr ){
        LOG(ERROR) << "create_image_surface() failed. " << SDL_GetError();
        return false;
    }

    return true;
}

void SDLApp::cleanup(){
    IMG_Quit();

    if ( m_imageSurface != nullptr ){
        SDL_FreeSurface(m_imageSurface);
        m_imageSurface = nullptr;
    }
    if ( m_screenRenderer != nullptr ){
        SDL_DestroyRenderer(m_screenRenderer);
        m_screenRenderer = nullptr;
    }
    if ( m_mainWindow != nullptr ){
        SDL_DestroyWindow(m_mainWindow);
        m_mainWindow = nullptr;
    }

    SDL_Quit();
}

void SDLApp::ToggleFullScreen(){
    if (m_fullscreen){
        SDL_SetWindowFullscreen(m_mainWindow, SDL_FALSE);
        m_fullscreen = false;
    } else {
        SDL_SetWindowFullscreen(m_mainWindow, SDL_TRUE);
        m_fullscreen = true;
    }
}

void SDLApp::Execute(){
    bool done = false;
    while ( !done ){
        SDL_Event event;
        while ( SDL_PollEvent(&event) != 0 ){
            if ( event.type == SDL_QUIT ){
                done = true;
            }
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE){
                    done = true;
                } else if (event.key.keysym.sym == SDLK_q){
                    //SDL_GetModState() & KMOD_CTRL
                    //inputText = SDL_GetClipboardText();
                    done = true;
                }
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                }
                break;
            case SDL_WINDOWEVENT:
                switch ( event.window.event ){
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    m_screenWidth = event.window.data1;
                    m_screenHeight = event.window.data2;

                    //SDL_DestroyRenderer(m_screenRenderer);
                    //m_screenRenderer = SDL_CreateRenderer(m_mainWindow, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);
                    //if ( m_screenRenderer == nullptr ){
                        //LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
                        // done = true;
                    //} else {

                    SDL_FreeSurface(m_imageSurface);
                    m_imageSurface = create_image_surface(m_screenWidth, m_screenHeight, m_screenBPP);
                    SDL_RenderPresent(m_screenRenderer);
                    // }
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_RenderPresent(m_screenRenderer);
                    break;

                //case SDL_WINDOWEVENT_ENTER:
                    //break;
                //case SDL_WINDOWEVENT_LEAVE:
                    //break;
                //case SDL_WINDOWEVENT_FOCUS_GAINED:
                    //break;
                //case SDL_WINDOWEVENT_FOCUS_LOST:
                    //break;
                //case SDL_WINDOWEVENT_MINIMIZED:
                    //break;
                //case SDL_WINDOWEVENT_MAXIMIZED:
                    //break;
                //case SDL_WINDOWEVENT_RESTORED:
                    //break;
                };
                break;
            default:
                break;
            };
            OnEvent(event, done);
        };

        Loop();

        RenderScreen();

        SDL_Delay(1); // Breath
    };
}

void SDLApp::RenderScreen() const {
    // Render(imageSurface)
    //SDL_Texture *backgroundTexture = SDL_CreateTexture(m_screenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_screenWidth, m_screenHeight);
    //if ( backgroundTexture == nullptr ){
        //LOG(ERROR) << "SDL_CreateTexture() failed. " << SDL_GetError();
        //return -1;
    //}
    //SDL_UpdateTexture(backgroundTexture, NULL, m_imageSurface->pixels, m_imageSurface->pitch);

    SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(m_screenRenderer, m_imageSurface);

    // Clear screen
    SDL_SetRenderDrawColor(m_screenRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(m_screenRenderer);
    // Render texture to screen
    SDL_Rect destRect{0, 0, (int)m_screenWidth, (int)m_screenHeight};
    SDL_RenderCopy(m_screenRenderer, backgroundTexture, NULL, &destRect);

    // Update screen
    SDL_RenderPresent(m_screenRenderer);

    SDL_DestroyTexture(backgroundTexture);
}

void SDLApp::OnEvent(SDL_Event event, bool &done){
}

void SDLApp::OnRender(cairo_surface_t *surface){
}

void SDLApp::RefreshScreen(uint64_t color) const{
    SDL_FillRect(m_imageSurface, NULL, color);
}

void SDLApp::Loop(){
    RefreshScreen(0xFFFFFF);

    cairo_surface_t *cairoSurface = create_cairo_surface_from_sdl_surface(m_imageSurface);
    if ( cairoSurface == nullptr ){
        LOG(ERROR) << "create_cairo_surface_from_sdl_surface() failed.";
        return;
    }

    OnRender(cairoSurface);

    cairo_surface_destroy(cairoSurface);
}

class MySDLApp : public SDLApp {
public:
    MySDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP, OFDDocumentPtr document);
    virtual ~MySDLApp();

    virtual void OnEvent(SDL_Event event, bool &done) override;
    virtual void OnRender(cairo_surface_t *surface) override;

    OFDDocumentPtr m_document;
    size_t m_pageIndex;

}; // class MySDLApp

MySDLApp::MySDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP, OFDDocumentPtr document)
    : SDLApp(title, screenWidth, screenHeight, screenBPP),
        m_document(document), m_pageIndex(0){
}

MySDLApp::~MySDLApp(){
}

// ======== MySDLApp::OnEvent() ========
void MySDLApp::OnEvent(SDL_Event event, bool &done){
    switch (event.type) {
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_UP) {
            if ( m_document != nullptr ){
                size_t totalPages = m_document->GetPagesCount();
                if ( totalPages > 0 ){
                    if ( m_pageIndex == 0 ){
                        m_pageIndex = totalPages - 1;
                    } else {
                        m_pageIndex--;
                    }
                }
            }
            break;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            if ( m_document != nullptr ){
                size_t totalPages = m_document->GetPagesCount();
                if ( totalPages > 0 ){
                    if ( m_pageIndex < totalPages - 1 ){
                        m_pageIndex++;
                    } else {
                        m_pageIndex = 0;
                    }
                }
            }
            break;
        } else if (event.key.keysym.sym == SDLK_RETURN) {
            ToggleFullScreen();
            break;
        }
        break;
    };
}

// ======== MySDLApp::OnRender() ========
void MySDLApp::OnRender(cairo_surface_t *surface){
    if ( m_document != nullptr ){
        size_t totalPages = m_document->GetPagesCount();
        if ( totalPages > 0 ){
            OFDPagePtr currentPage = m_document->GetOFDPage(m_pageIndex);
            if ( currentPage->Open() ){
                currentPage->Render(surface);
            } else {
                LOG(ERROR) << "currentPage->Open() failed. pageIndex=" << m_pageIndex;
            }
        }
    }
}


bool fullScreen = false;

// -------- loadTexture() --------
SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &image_file){
    SDL_Texture *texture = nullptr;

    SDL_Surface *loadedSurface = IMG_Load(image_file.c_str());
    if ( loadedSurface != nullptr ){
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if ( texture == nullptr ){
            LOG(WARNING) << "SDL_CreateTextureFromSurface() failed. " << SDL_GetError();
        }
        SDL_FreeSurface(loadedSurface);
    } else {
        LOG(WARNING) << "IMG_Load(\"" << image_file << "\") failed " << SDL_GetError();
    }

    return texture;
}

int main(int argc, char *argv[]){

    TIMED_FUNC(timerMain);

    Logger::Initialize(argc, argv);

    LOG(INFO) << "Start " << argv[0];

    std::string filename = argv[1];

    OFDPackage package;
    if ( !package.Open(filename) ){
        LOG(ERROR) << "OFDPackage::Open() failed. filename:" << filename;
        return -1;
    }
    OFDDocumentPtr document = OFDDocumentPtr(package.GetOFDDocument()); 
    assert(document != nullptr);
    LOG(DEBUG) << document->String();

    size_t total_pages = document->GetPagesCount();
    LOG(INFO) << total_pages << " pages in " << filename;
    if ( total_pages > 0 ){
        int screenWidth = 794;
        int screenHeight = 1122;
        int screenBPP = 32;

        MySDLApp app("OFDViewer", screenWidth, screenHeight, screenBPP, document);
        
        app.Execute();
    }

    package.Close();



    LOG(INFO) << "Done.";

    return 0;
}

//int main(){
int main_old(int argc, char *argv[]){

    TIMED_FUNC(timerMain);

    Logger::Initialize(argc, argv);

    LOG(INFO) << "Start " << argv[0];


    OFDPackage package;
    OFDDocumentPtr document = nullptr;
    OFDPagePtr currentPage = nullptr;
    size_t totalPages = 0;
    size_t showPage = 0;
    if ( package.Open(argv[1]) ){
        document = OFDDocumentPtr(package.GetOFDDocument()); 
        //OFDDocumentPtr document = package.GetOFDDocument(); 

        LOG(DEBUG) << document->String();

        size_t n_pages = document->GetPagesCount();
        totalPages = n_pages;
        LOG(INFO) << "Loading " << n_pages << " pages.";

        //for ( size_t i = 0 ; i < n_pages ; i++ ){
            //TIMED_SCOPE(timerDrawPage, "Draw Page");

            //OFDPagePtr page = document->GetOFDPage(i);
            ////page = document->GetOFDPage(i);
            //page->Open();

            //VLOG(3) << page->String(); 
            //LOG(INFO) << page->GetText();

            ////std::stringstream ss;
            ////ss << "Page" << (i + 1) << ".png";
            ////std::string png_filename = ss.str();
            ////page->RenderToPNGFile(png_filename);

            ////page->Close();
        //}
        currentPage = document->GetOFDPage(0);
        currentPage->Open();

    } else {
        LOG(ERROR) << "package.Open() failed. " << argv[1];
        return -1;
    }


    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ){
        LOG(ERROR) << "Failed to initialize SDL. " << SDL_GetError();
        return -1;
    }

    if ( !SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") ){
        LOG(ERROR) << "SDL_SetHint() failed. " << SDL_GetError();
        return -1;
    }

    int screenWidth = 794;
    int screenHeight = 1122;
    int screenFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE; // SDL_WINDOW_BORDERLESS
    int screenBPP = 32;
    SDL_Window *mainWindow = SDL_CreateWindow("OFD Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, screenFlags);
    if ( mainWindow == nullptr ){
        LOG(ERROR) << "SDL_CreateWindow() failed. " << SDL_GetError();
        return -1;
    }

    //SDL_Surface *primarySurface = SDL_GetWindowSurface(mainWindow);
    //cairo_surface_t *cairoSurface = create_cairo_surface_from_sdl_surface(primarySurface);
    //if ( cairoSurface == nullptr ){
        //LOG(ERROR) << "create_cairo_surface_from_sdl_surface() failed.";
        //return -1;
    //}

    SDL_Renderer *screenRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
    if ( screenRenderer == nullptr ){
        LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
        return -1;
    }

    if ( !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ){
        LOG(ERROR) << "IMG_Init() failed. " << SDL_GetError();
        return -1;
    }


    SDL_Surface *imageSurface = create_image_surface(screenWidth, screenHeight, screenBPP);

    bool done = false;
    while ( !done ){
        SDL_Event event;
        while ( SDL_PollEvent(&event) != 0 ){
            if ( event.type == SDL_QUIT ){
                done = true;
            }
            // OnEvent(event);
            switch (event.type) {
            //case SDL_TEXTINPUT:
                //break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE){
                    done = true;
                } else if (event.key.keysym.sym == SDLK_q){
                    //SDL_GetModState() & KMOD_CTRL
                    //inputText = SDL_GetClipboardText();
                    done = true;
                } else if (event.key.keysym.sym == SDLK_UP) {
                    if ( showPage == 0 ){
                        showPage = totalPages - 1;
                    } else {
                        showPage--;
                    }
                    currentPage = document->GetOFDPage(showPage);
                    currentPage->Open();
                    break;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    if ( showPage < totalPages - 1 ){
                        showPage++;
                    } else {
                        showPage = 0;
                    }
                    currentPage = document->GetOFDPage(showPage);
                    currentPage->Open();
                    break;
                } else if (event.key.keysym.sym == SDLK_RETURN) {
                    if (fullScreen){
                        SDL_SetWindowFullscreen(mainWindow, SDL_FALSE);
                        fullScreen = false;
                    } else {
                        SDL_SetWindowFullscreen(mainWindow, SDL_TRUE);
                        fullScreen = true;
                    }
                }
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                }
                break;
            case SDL_WINDOWEVENT:
                switch ( event.window.event ){
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screenWidth = event.window.data1;
                    screenHeight = event.window.data2;

                    //SDL_DestroyRenderer(screenRenderer);
                    //screenRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);
                    //if ( screenRenderer == nullptr ){
                        //LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
                        //return -1;
                    //}

                    SDL_FreeSurface(imageSurface);
                    imageSurface = create_image_surface(screenWidth, screenHeight, screenBPP);
                    SDL_RenderPresent(screenRenderer);
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_RenderPresent(screenRenderer);
                    break;

                case SDL_WINDOWEVENT_ENTER:
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    break;
                };
                break;
            };
        };

        // Loop()
        SDL_FillRect(imageSurface, NULL, 0xFFFFFF);


        cairo_surface_t *cairoSurface = create_cairo_surface_from_sdl_surface(imageSurface);
        if ( cairoSurface == nullptr ){
            LOG(ERROR) << "create_cairo_surface_from_sdl_surface() failed.";
            return -1;
        }

        if ( currentPage != nullptr ){
            currentPage->Render(cairoSurface);
        }

        // Render(imageSurface)
        //SDL_Texture *backgroundTexture = SDL_CreateTexture(screenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);
        //if ( backgroundTexture == nullptr ){
            //LOG(ERROR) << "SDL_CreateTexture() failed. " << SDL_GetError();
            //return -1;
        //}
        //SDL_UpdateTexture(backgroundTexture, NULL, imageSurface->pixels, imageSurface->pitch);

        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(screenRenderer, imageSurface);

        // Clear screen
        SDL_SetRenderDrawColor(screenRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(screenRenderer);
        // Render texture to screen
        SDL_Rect destRect{0, 0, screenWidth, screenHeight};
        SDL_RenderCopy(screenRenderer, backgroundTexture, NULL, &destRect);

        // Update screen
        SDL_RenderPresent(screenRenderer);

        SDL_DestroyTexture(backgroundTexture);

        SDL_Delay(1); // Breath
    };

    // Cleanup()
    SDL_FreeSurface(imageSurface);
    if ( screenRenderer != nullptr ){
        SDL_DestroyRenderer(screenRenderer);
        screenRenderer = nullptr;
    }

    if ( mainWindow != nullptr ){
        SDL_DestroyWindow(mainWindow);
        mainWindow = nullptr;
    }

    IMG_Quit();
    SDL_Quit();

    package.Close();

    LOG(INFO) << "Done.";

    return 0;
}

