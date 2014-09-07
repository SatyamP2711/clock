/*-------------------------------------------
  pagecolor.c
  "Color and Font" page
  KAZUBON 1997-1998
---------------------------------------------*/
// Modified by Stoic Joker: Monday, March 8 2010 - 7:52:55
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void InitColor(HWND hDlg);
static void InitComboFont(HWND hDlg);
static void OnChooseColor(HWND hDlg, WORD id);
static void OnDrawItemColorCombo(LPARAM lParam);
static void SetComboFontSize(HWND hDlg, int bInit);
static void OnMeasureItemColorCombo(LPARAM lParam);
static HFONT hfontb;  // for IDC_BOLD
static HFONT hfonti;  // for IDC_ITALIC

static const char* g_rotate[]={
	"None",
	"Left",
	"Right",
};
static const int g_rotateCount=sizeof(g_rotate)/sizeof(char*);

static __inline void SendPSChanged(HWND hDlg){
	g_bApplyTaskbar = TRUE;
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageColorProc(HWND hDlg, UINT message,
							WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_MEASUREITEM:
		OnMeasureItemColorCombo(lParam);
		return TRUE;
	case WM_DRAWITEM:
		OnDrawItemColorCombo(lParam);
		return TRUE;
	case WM_COMMAND: {
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			if((id == IDC_COLFORE || id == IDC_FONT || id == IDC_FONTQUAL ||
				id == IDC_FONTSIZE || id == IDC_CLOCKROTATE) && code == CBN_SELCHANGE) {
				if(id == IDC_FONT) SetComboFontSize(hDlg, FALSE);
				SendPSChanged(hDlg);
			} else if(id == IDC_CHOOSECOLFORE)
				OnChooseColor(hDlg, id);
			else if(id == IDC_BOLD || id == IDC_ITALIC)
				SendPSChanged(hDlg);
			else if((id == IDC_CLOCKHEIGHT || id == IDC_CLOCKWIDTH || id == IDC_ALPHATB ||
					 id == IDC_VERTPOS || id == IDC_LINEHEIGHT || id == IDC_HORIZPOS)
					&& code == EN_CHANGE)
				SendPSChanged(hDlg);
			return TRUE;
		}
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY: OnApply(hDlg); break;
		}
		return TRUE;
	case WM_DESTROY:
		DeleteObject(hfontb);
		DeleteObject(hfonti);
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}

/*------------------------------------------------
  Initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HDC hdc;
	LOGFONT logfont;
	HFONT hfont;
	char buf[1024];
	int i;
	
	// setting of "background" and "text"
	InitColor(hDlg);
	
	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8) {
		EnableDlgItem(hDlg, IDC_CHOOSECOLFORE, FALSE);
	}
	DeleteDC(hdc);
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_FONT, WM_SETFONT, (WPARAM)hfont, 0);
		
		
	InitComboFont(hDlg);
	
	SetComboFontSize(hDlg, TRUE);
	
	
	CheckDlgButton(hDlg, IDC_BOLD, GetMyRegLong("Clock", "Bold", TRUE));
	CheckDlgButton(hDlg, IDC_ITALIC, GetMyRegLong("Clock", "Italic", FALSE));
	
	hfontb = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfontb, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = FW_BOLD;
	hfontb = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_BOLD, WM_SETFONT, (WPARAM)hfontb, 0);
	
	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = 1;
	hfonti = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_ITALIC, WM_SETFONT, (WPARAM)hfonti, 0);
	
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETRANGE32, (WPARAM)-999,999);
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETPOS32, 0, GetMyRegLong("Clock", "ClockHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETRANGE32, (WPARAM)-999,999);
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETPOS32, 0, GetMyRegLong("Clock", "ClockWidth", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETRANGE32, (WPARAM)-64,64);
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETPOS32, 0, GetMyRegLong("Clock", "LineHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETPOS32, 0, GetMyRegLong("Clock", "VertPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETPOS32, 0, GetMyRegLong("Clock", "HorizPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETRANGE32, 0,180);
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETPOS32, 0, GetMyRegLong("Taskbar", "AlphaTaskbar", 0));
	
	for(i=0; i<g_rotateCount; ++i)
		CBAddString(hDlg,IDC_CLOCKROTATE,g_rotate[i]);
	GetMyRegStr("Clock","FontRotateDirection",buf,80,g_rotate[0]);
	if(_strnicmp(buf,g_rotate[1],4)==0)
		CBSetCurSel(hDlg,IDC_CLOCKROTATE,1);
	else if(_strnicmp(buf,g_rotate[2],5)==0)
		CBSetCurSel(hDlg,IDC_CLOCKROTATE,2);
	else
		CBSetCurSel(hDlg,IDC_CLOCKROTATE,0);
	i=GetMyRegLong("Clock","FontQuality",CLEARTYPE_QUALITY);
	CBAddString(hDlg,IDC_FONTQUAL,"Default");				// DEFAULT_QUALITY			 = 0
	CBAddString(hDlg,IDC_FONTQUAL,"Draft");					// DRAFT_QUALITY			 = 1
	CBAddString(hDlg,IDC_FONTQUAL,"Proof");					// PROOF_QUALITY			 = 2
	CBAddString(hDlg,IDC_FONTQUAL,"NonAntiAliased");		// NONANTIALIASED_QUALITY	 = 3
	CBAddString(hDlg,IDC_FONTQUAL,"AntiAliased (Win7)");	// ANTIALIASED_QUALITY		 = 4
	CBAddString(hDlg,IDC_FONTQUAL,"ClearType (WinXP+)");	// CLEARTYPE_QUALITY		 = 5
	CBAddString(hDlg,IDC_FONTQUAL,"ClearType Natural");		// CLEARTYPE_NATURAL_QUALITY = 6
	CBSetCurSel(hDlg,IDC_FONTQUAL,i);
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;
	char s[80];
	char ss[1024];
	
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLFORE, CBGetCurSel(hDlg, IDC_COLFORE));
	SetMyRegLong("Clock", "ForeColor", dw);
	
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), s);
	SetMyRegStr("Clock", "Font", s);
	
	if(CBGetCount(hDlg, IDC_FONTSIZE) > 0) {
		CBGetLBText(hDlg, IDC_FONTSIZE, CBGetCurSel(hDlg, IDC_FONTSIZE), s);
		SetMyRegLong("Clock", "FontSize", atoi(s));
	} else SetMyRegLong("Clock", "FontSize", 9);
	
	SetMyRegLong("Clock", "Bold",   IsDlgButtonChecked(hDlg, IDC_BOLD));
	SetMyRegLong("Clock", "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
	
	SetMyRegLong("Clock", "FontQuality", (DWORD)CBGetCurSel(hDlg, IDC_FONTQUAL));
	
	SetMyRegLong("Clock", "ClockHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINCHEIGHT,UDM_GETPOS32,0,0));
	SetMyRegLong("Clock", "ClockWidth", (int)SendDlgItemMessage(hDlg,IDC_SPINCWIDTH,UDM_GETPOS32,0,0));
	SetMyRegLong("Clock", "LineHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINLHEIGHT,UDM_GETPOS32,0,0));
	SetMyRegLong("Clock", "VertPos", (int)SendDlgItemMessage(hDlg,IDC_SPINVPOS,UDM_GETPOS32,0,0));
	SetMyRegLong("Clock", "HorizPos", (int)SendDlgItemMessage(hDlg,IDC_SPINHPOS,UDM_GETPOS32,0,0));
	SetMyRegLong("Taskbar","AlphaTaskbar",(int)SendDlgItemMessage(hDlg,IDC_SPINALPHA,UDM_GETPOS32,0,0));
	
	GetDlgItemText(hDlg,IDC_CLOCKROTATE,ss,1024);
	if(_strnicmp(ss,g_rotate[1],4)==0)			SetMyRegStr("Clock","FontRotateDirection",g_rotate[1]);
	else if(_strnicmp(ss,g_rotate[2],5)==0)		SetMyRegStr("Clock","FontRotateDirection",g_rotate[2]);
	else										SetMyRegStr("Clock","FontRotateDirection",g_rotate[0]);
}

/*------------------------------------------------
--------------------------------------------------*/
void InitColor(HWND hDlg)
{
	COLORREF col;
	int j;
	WORD id;
	//Windowsデフォルト16色
	int rgb[16][3] = {{0,0,0}, {128,0,0}, {0,128,0}, {128,128,0},
		{0,0,128}, {128,0,128}, {0,128,128}, {192,192,192},
		{128,128,128}, {255,0,0}, {0,255,0}, {255,255,0},
		{0,0,255},{255,0,255}, {0,255,255}, {255,255,255}
	};
	
	id = IDC_COLFORE;
	
	for(j = 0; j < 16; j++) //基本16色
		CBAddString(hDlg, id,
					RGB(rgb[j][0], rgb[j][1], rgb[j][2]));
	//ボタンの...色
	CBAddString(hDlg, id, 0x80000000|COLOR_3DFACE);
	CBAddString(hDlg, id, 0x80000000|COLOR_3DSHADOW);
	CBAddString(hDlg, id, 0x80000000|COLOR_3DHILIGHT);
	CBAddString(hDlg, id, 0x80000000|COLOR_BTNTEXT);
	
	// New Default Font Color is White - Better Contrast for Vista & 7
	col = GetMyRegLong("Clock", "ForeColor", 0x00ffffff); // 0x80000000 | COLOR_BTNTEXT);
	
	for(j = 0; j < 20; j++) {
		if(col == (COLORREF)CBGetItemData(hDlg, id, j))
			break;
	}
	if(j == 20) // Add the Selected Custom Color
		CBAddString(hDlg, id, col);
		
	CBSetCurSel(hDlg, id, j);
}

/*------------------------------------------------
--------------------------------------------------*/
void OnMeasureItemColorCombo(LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pmis;
	pmis = (LPMEASUREITEMSTRUCT)lParam;
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
}

/*------------------------------------------------
--------------------------------------------------*/
void OnDrawItemColorCombo(LPARAM lParam)
{
	LPDRAWITEMSTRUCT pdis;
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	
	pdis = (LPDRAWITEMSTRUCT)lParam;
	
	if(IsWindowEnabled(pdis->hwndItem)) {
		col = (COLORREF)(ULONG_PTR)pdis->itemData;
		if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	} else col = GetSysColor(COLOR_3DFACE);
	
	switch(pdis->itemAction) {
	case ODA_DRAWENTIRE:
	case ODA_SELECT: {
			hbr = CreateSolidBrush(col);
			FillRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
			
			// print color names
			if(16 <= pdis->itemID && pdis->itemID <= 19) {
				char s[80];
				int y;
				strcpy(s, MyString(IDS_BTNFACE + pdis->itemID - 16));
				SetBkMode(pdis->hDC, TRANSPARENT);
				GetTextMetrics(pdis->hDC, &tm);
				if(pdis->itemID == 19)
					SetTextColor(pdis->hDC, RGB(255,255,255));
				else
					SetTextColor(pdis->hDC, RGB(0,0,0));
				y = (pdis->rcItem.bottom - pdis->rcItem.top - tm.tmHeight)/2;
				TextOut(pdis->hDC, pdis->rcItem.left + 4, pdis->rcItem.top + y,
						s, (int)strlen(s));
			}
			if(!(pdis->itemState & ODS_FOCUS)) break;
		}
	case ODA_FOCUS: {
			if(pdis->itemState & ODS_FOCUS)
				hbr = CreateSolidBrush(0);
			else
				hbr = CreateSolidBrush(col);
			FrameRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
			break;
		}
	}
}

/*------------------------------------------------
--------------------------------------------------*/
void OnChooseColor(HWND hDlg, WORD id)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	WORD idCombo;
	int i;
	
	idCombo = id - 1;
	
	col = (COLORREF)CBGetItemData(hDlg, idCombo, CBGetCurSel(hDlg, idCombo));
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	
	for(i = 0; i < 16; i++) colarray[i] = RGB(255,255,255);
	
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hDlg;
	cc.hInstance = (HWND)(HINSTANCE)GetModuleHandle(NULL);
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc)) return;
	
	for(i = 0; i < 16; i++) {
		if(cc.rgbResult == (COLORREF)CBGetItemData(hDlg, idCombo, i))
			break;
	}
	if(i == 16) { //基本16色ではないとき
		if(CBGetCount(hDlg, idCombo) == 20)
			CBAddString(hDlg, idCombo, cc.rgbResult);
		else
			CBSetItemData(hDlg, idCombo, 20, cc.rgbResult);
		i = 20;
	}
	CBSetCurSel(hDlg, idCombo, i);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);
int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);
int logpixelsy;
/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFont(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf;
	HWND hcombo;
	char s[80];
	int i;
	
	hdc = GetDC(NULL);
	
	// Enumerate fonts and set in the combo box
	memset(&lf, 0, sizeof(LOGFONT));
	hcombo = GetDlgItem(hDlg, IDC_FONT);
	
	lf.lfCharSet = (BYTE)GetTextCharset(hdc);  // MS UI Gothic, ...
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	
	lf.lfCharSet = OEM_CHARSET;   // Small Fonts, Terminal...
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	
	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	
	
	ReleaseDC(NULL, hdc);
	
	GetMyRegStrEx("Clock", "Font", s, 80, "Arial");
	
	i = (int)CBFindStringExact(hDlg, IDC_FONT, s);
	if(i == LB_ERR) i = 0;
	CBSetCurSel(hDlg, IDC_FONT, i);
}

/*------------------------------------------------
--------------------------------------------------*/
void SetComboFontSize(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	char str[160];
	DWORD size;
	LOGFONT lf;
	
	//以前のsizeを保存
	if(bInit) { // WM_INITDIALOGのとき
		size = GetMyRegLong("Clock", "FontSize", 10);
		if(size<=0) size = 10;
	} else { // IDC_FONTが変更されたとき
		CBGetLBText(hDlg, IDC_FONTSIZE,
					CBGetCurSel(hDlg, IDC_FONTSIZE), str);
		size = atoi(str);
	}
	
	CBResetContent(hDlg, IDC_FONTSIZE);
	
	hdc = GetDC(NULL);
	logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), str);
	
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, str);
	lf.lfCharSet = (BYTE)CBGetItemData(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT));
	EnumFontFamiliesEx(hdc, &lf, EnumSizeProcEx,
					   (LPARAM)GetDlgItem(hDlg, IDC_FONTSIZE), 0);
					   
	ReleaseDC(NULL, hdc);
	
	for(; size; --size) {
		int pos;
		wsprintf(str, "%d", size);
		pos = (int)CBFindStringExact(hDlg, IDC_FONTSIZE, str);
		if(pos != LB_ERR) {
			CBSetCurSel(hDlg, IDC_FONTSIZE, pos); return;
		}
	}
	CBSetCurSel(hDlg, IDC_FONTSIZE, 0);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	(void)lpntme; (void)FontType;
	if(lpelfe->lfFaceName[0]!='@' && SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, 0, (LPARAM)lpelfe->lfFaceName)==LB_ERR) {
		int index = (int)SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelfe->lfFaceName);
		if(index >= 0)
			SendMessage((HWND)lParam, CB_SETITEMDATA, index, lpelfe->lfCharSet);
	}
	return 1;
}

/*------------------------------------------------
--------------------------------------------------*/
int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	const unsigned char nFontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,18,20,22,24,26,28,36,48,72};
	char str[8];
	int i;
	
	(void)lpelfe;
	
	if((FontType & TRUETYPE_FONTTYPE) ||
	   !((FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE))) {
		for(i=0; i<sizeof(nFontSizes); ++i) {
			wsprintf(str,"%hu",nFontSizes[i]);
			SendMessage((HWND)lParam,CB_ADDSTRING,0,(LPARAM)str);
		}
		return FALSE;
	}else{
		int num = (lpntme->tmHeight - lpntme->tmInternalLeading) * 72 / logpixelsy;
		int count = (int)SendMessage((HWND)lParam, CB_GETCOUNT, 0, 0);
		for(i = 0; i < count; ++i) {
			SendMessage((HWND)lParam, CB_GETLBTEXT, i, (LPARAM)str);
			if(num == atoi(str)) return TRUE;
			else if(num < atoi(str)) {
				wsprintf(str, "%d", num);
				SendMessage((HWND)lParam, CB_INSERTSTRING, i, (LPARAM)str);
				return TRUE;
			}
		}
		wsprintf(str, "%d", num);
		SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)str);
		return TRUE;
	}
}
