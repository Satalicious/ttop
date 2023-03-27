//
// generic utility function to retrieve a URL via http or https
// and return a wxString utilizing OpenSSL libraries
//

#include "fetchurl.h"

struct MemoryStruct {
	char *memory;
	size_t size;
};

static void *myrealloc(void *ptr , size_t size);

static void *myrealloc(void *ptr , size_t size ) {
    if (ptr) 
	    return realloc(ptr,size);
    else
        return malloc(size);
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) data;
    mem->memory = (char *) myrealloc(mem->memory , mem->size + realsize + 1);
    if (mem->memory) {
	    memcpy(&(mem->memory[mem->size]),ptr,realsize);
	    mem->size += realsize;
	    mem->memory[mem->size] = 0;
    }
    return realsize;
}

wxString FetchURL( wxURI targetUrl , wxString & cType ) {

	wxString   fetchedData;
	// char       contentType[50];
	char *contentType = NULL;
	char       charUrl[1024];
	char       windowsProxy[1024];
	char       charError[CURL_ERROR_SIZE];
	wxString   strUrl;

	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.memory = NULL;
	chunk.size = 0;

	// check for proxy settings
	strncpy( windowsProxy , GetWindowsProxy().c_str() , 1024 );

	strncpy( charUrl , targetUrl.BuildURI().ToAscii() , 1024 );

         // new curl code to handle http and https
	curl = curl_easy_init();
	if (curl) {
	     curl_easy_setopt(curl,CURLOPT_URL, charUrl ); 
	     // curl_easy_setopt(curl,CURLOPT_ERRORBUFFER , charError );
	     curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER , 0L);
	     curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST , 0L);
             curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION , WriteMemoryCallback);
	     curl_easy_setopt(curl,CURLOPT_WRITEDATA, (void *)&chunk);
	     if ( strlen(windowsProxy) > 0 ) {
	         curl_easy_setopt(curl,CURLOPT_PROXY, &windowsProxy );
             }
	     curl_easy_setopt(curl,CURLOPT_USERAGENT,"plpprint/0.9");
	     res = curl_easy_perform(curl);
	     // if ( curl_easy_getinfo(curl,CURLINFO_CONTENT_TYPE,contentType) == CURLE_OK ) {
	     curl_easy_getinfo(curl,CURLINFO_CONTENT_TYPE, &contentType);
	     cType = wxString::FromAscii(contentType); 
	     //} else {
             // wxMessageBox( wxT("Bork!") , wxT("Data") , wxOK, NULL );
	     //	 cType = wxT("");
	     //}
         
         curl_easy_cleanup(curl);
    }
	
    // wxMessageBox( wxT(chunk.memory) , wxT("Data") , wxOK, NULL );
    // if ( cType == wxT("text/html") ) {
    //    fetchedData = wxString( chunk.memory , wxConvUTF8 , chunk.size );
    //} else {
	    fetchedData = wxString::From8BitData( chunk.memory , chunk.size );
    //}
    // fetchedData = wxString( chunk.memory , wxConvUTF8 , chunk.size );
    wxLogMessage( "Fetched Data Size: (%d)",chunk.size);
    wxLogMessage( "Fetched Data: " + fetchedData );

    if ( chunk.memory) 
        free(chunk.memory);

    return fetchedData;
}

wxString GetWindowsProxy( void ) {
    wxString proxyInfo;
    long     proxyEnabled;

    wxRegKey proxyKey(wxRegKey::HKCU, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
    proxyKey.QueryValue("ProxyEnable",&proxyEnabled);
    if ( proxyEnabled == 1 ) {
        proxyKey.QueryValue("ProxyServer",proxyInfo);
	} else {
	    proxyInfo = "";
	}
    return proxyInfo;
}


