// Isolated 0.2.5 transfer experiment; no accepted coverage.
#include <windows.h>
#define MAX_TEXT_DRAWS 2304
// Destructor declaration only: actual R5 provider remains required.
class CTextDraw { public: ~CTextDraw(); };
class CTextDrawPool {
    BOOL m_bSlotState[MAX_TEXT_DRAWS];
    CTextDraw* m_pTextDraw[MAX_TEXT_DRAWS];
public:
    ~CTextDrawPool();
    void Delete(WORD wText);
};
CTextDrawPool::~CTextDrawPool()
{
	int x=0;
	while(x != MAX_TEXT_DRAWS) {
		if(m_pTextDraw[x]) {
			delete m_pTextDraw[x];
			m_pTextDraw[x] = NULL;
			m_bSlotState[x] = FALSE;
		}
		x++;
	}
}
void CTextDrawPool::Delete(WORD wText)
{
	if(m_pTextDraw[wText]) {
		delete m_pTextDraw[wText];
		m_pTextDraw[wText] = NULL;
		m_bSlotState[wText] = FALSE;
	}	
}
