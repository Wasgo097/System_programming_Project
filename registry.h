#ifndef REGISTRY_H
#define REGISTRY_H
#include <windows.h>
class Registry{
private:
   HKEY uchwytKlucza;
   HKEY kluczGlowny;
   REGSAM trybDostepu;
public:
   bool wyswietlajKomunikatyOBledach;
public:
    Registry(HKEY kluczGlowny,REGSAM trybDostepu,bool wyswietlajKomunikatyOBledach=true);
    ~Registry(){}
    void WyswietlKomunikatBledu(LONG numerBledu) const;
    bool UtworzKlucz(const wchar_t* klucz);
    bool OtworzKlucz(const wchar_t* klucz);
    bool ZamknijKlucz();
    bool UsunKlucz(const wchar_t* klucz);
    bool CzyKluczIstnieje(const wchar_t* klucz) const;
    bool OtworzKlucz(const wchar_t* klucz,bool utworzJezeliNieIstnieje);
    bool CzyWartoscIstnieje(const wchar_t* nazwa) const;
    bool UsunWartosc(const wchar_t* nazwa) const;
    int CzytajInt(const wchar_t* nazwa) const;
    bool ZapiszInt(const wchar_t* nazwa,int wartosc) const;
    wchar_t* CzytajString(const wchar_t* nazwa,wchar_t* wartosc) const;
    bool ZapiszString(const wchar_t* nazwa,const wchar_t* zawartosc) const;
};
#endif // REGISTRY_H
