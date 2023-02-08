#include "wstr.h"
#include <wchar.h>  // required for wchar_t
#include <assert.h>  // required for assert()

static void AssertWStrArrEqual(_In_ const struct WStrArr  *wstrArr,
                               _In_ const wchar_t        **lppExpectedArr,
                               _In_ const size_t           ulExpectedCount)
{
    assert(ulExpectedCount == wstrArr->ulSize);
    if (0 == ulExpectedCount)
    {
        assert(NULL == lppExpectedArr);
        assert(NULL == wstrArr->lpWStrArr);
    }
    else
    {
        for (size_t i = 0; i < ulExpectedCount; ++i)
        {
            if (L'\0' == lppExpectedArr[i][0])
            {
                assert(NULL == wstrArr->lpWStrArr[i].lpWCharArr);
            }
            else
            {
                assert(0 == wcscmp(lppExpectedArr[i], wstrArr->lpWStrArr[i].lpWCharArr));
            }
            assert(wcslen(lppExpectedArr[i]) == wstrArr->lpWStrArr[i].ulSize);
        }
    }
}

/**
 * @param formatConst
 *        Ex: "%d"
 */
#define AssertEqual(/* char * */ formatConst,  \
                    /* char * */ leftDesc,     \
                    /* (any)  */ leftValue,    \
                    /* char * */ rightDesc,    \
                    /* (any)  */ rightValue)   \
    do { \
        if (!(leftValue == rightValue)) \
        { \
            char buf[1024] = {0}; \
            snprintf(buf, sizeof(buf), "%s[%s] != %s[%s]\r\n", leftDesc, formatConst, rightDesc, formatConst); \
            printf(buf, leftValue, rightValue); \
            assert(leftValue == rightValue); \
        } \
    } while(0)

/**
 * @param formatConst
 *        Ex: "%d"
 */
#define AssertLess(/* char * */ formatConst,  \
                   /* char * */ leftDesc,     \
                   /* (any)  */ leftValue,    \
                   /* char * */ rightDesc,    \
                   /* (any)  */ rightValue)   \
    do { \
        if (!(leftValue < rightValue)) \
        { \
            char buf[1024] = {0}; \
            snprintf(buf, sizeof(buf), "%s[%s] >= %s[%s]\r\n", leftDesc, formatConst, rightDesc, formatConst); \
            printf(buf, leftValue, rightValue); \
            assert(leftValue < rightValue); \
        } \
    } while(0)

/**
 * @param formatConst
 *        Ex: "%d"
 */
#define AssertGreater(/* char * */ formatConst,  \
                      /* char * */ leftDesc,     \
                      /* (any)  */ leftValue,    \
                      /* char * */ rightDesc,    \
                      /* (any)  */ rightValue)   \
    do { \
        if (!(leftValue > rightValue)) \
        { \
            char buf[1024] = {0}; \
            snprintf(buf, sizeof(buf), "%s[%s] <= %s[%s]\r\n", leftDesc, formatConst, rightDesc, formatConst); \
            printf(buf, leftValue, rightValue); \
            assert(leftValue < rightValue); \
        } \
    } while(0)

