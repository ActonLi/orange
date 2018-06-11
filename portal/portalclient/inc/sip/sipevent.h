/******************************************************************************
* Copyright 2010-2011 ABB Genway Co.,Ltd.
* FileName:       sipevent.h
* Desc:
* 
* 
* Author:    daniel_qinghua.huang
* Date:      2015/03/10
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2015/03/10, daniel_qinghua.huang create this file
* 
******************************************************************************/

#ifndef __SIPEVENT_H__
#define __SIPEVENT_H__

//-----------------------------------------
#define SIP_EXT_100REL                  /* rfc3262 */
#define SIP_EXT_PRIVACY_ID              /* rfc3325 */
#define SIP_EXT_REMOTE_PRIVACY          /* draft-ietf-sip-privacy */
//#define SIP_EXT_SESSION_TIMER           /* draft-ietf-sip-session-timer */
#define SIP_EXT_CC_TRANSFER             /* rfc3515, rfc3265 */
#define SIP_EXT_REPLACES                /* rfc3891 */


//-----------------------------------------
typedef enum type_t {
    /* TIMEOUT EVENTS for ICT */
    TIMEOUT_A,            /**< Timer A */
    TIMEOUT_B,            /**< Timer B */
    TIMEOUT_D,            /**< Timer D */

    /* TIMEOUT EVENTS for NICT */
    TIMEOUT_E,            /**< Timer E */
    TIMEOUT_F,            /**< Timer F */
    TIMEOUT_K,            /**< Timer K */

    /* TIMEOUT EVENTS for IST */
    TIMEOUT_G,            /**< Timer G */
    TIMEOUT_H,            /**< Timer H */
    TIMEOUT_I,            /**< Timer I */

    /* TIMEOUT EVENTS for NIST */
    TIMEOUT_J,            /**< Timer J */

    /* FOR INCOMING MESSAGE */
    RCV_REQINVITE,        /**< Event is an incoming INVITE request */
    RCV_REQACK,           /**< Event is an incoming ACK request */
    RCV_REQUEST,          /**< Event is an incoming NON-INVITE and NON-ACK request */
    RCV_STATUS_1XX,       /**< Event is an incoming informational response */
    RCV_STATUS_2XX,       /**< Event is an incoming 2XX response */
    RCV_STATUS_3456XX,    /**< Event is an incoming final response (not 2XX) */

    /* FOR OUTGOING MESSAGE */
    SND_REQINVITE,        /**< Event is an outgoing INVITE request */
    SND_REQACK,           /**< Event is an outgoing ACK request */
    SND_REQUEST,          /**< Event is an outgoing NON-INVITE and NON-ACK request */
    SND_STATUS_1XX,       /**< Event is an outgoing informational response */
    SND_STATUS_2XX,       /**< Event is an outgoing 2XX response */
    SND_STATUS_3456XX,    /**< Event is an outgoing final response (not 2XX) */

    KILL_TRANSACTION,     /**< Event to 'kill' the transaction before termination */
    UNKNOWN_EVT
} type_t;


//-----------------------------------------
typedef struct node_t node_t;
struct node_t {
    void        *next;       /* next node_t */
    void        *element;
};

typedef struct list_t list_t;
struct list_t {
    int         nb_elt;
    node_t      *node;
};

typedef struct url_t url_t;
struct url_t {
    char        *scheme;
    char        *username;
    char        *password;
    char        *host;
    char        *port;
    list_t      *url_params;
    list_t      *url_headers;
    char        *string;   /** other url schemes are strings. (http, mailto...) */
};

typedef struct startline_t startline_t;
struct startline_t {
    /* msgevttype_t method; */
    char        *sipmethod;
    char        *sipversion;
    /* req */
    url_t       *rquri;
    /* resp */
    char        *statuscode;
    char        *reasonphrase;
};


typedef struct call_id_t call_id_t;
struct call_id_t {
    char        *number;
    char        *host;
};

typedef struct content_length_t content_length_t;
struct content_length_t {
    char        *value;
};

typedef struct content_type_t content_type_t;
struct content_type_t {
    char        *type;
    char        *subtype;
    list_t      *gen_params;
};

typedef struct cseq_t cseq_t;
struct cseq_t {
    char        *method;
    char        *number;
};

typedef struct from_t from_t;
struct from_t {
    char        *displayname;
    url_t       *url;  /* could contain various urischeme_t ? only in the future */
    list_t      *gen_params;
};

#ifdef SIP_EXT_REMOTE_PRIVACY
typedef from_t remotepartyid_t;
#endif

typedef from_t to_t;

typedef content_length_t mime_version_t;

#ifdef SIP_EXT_100REL
typedef struct rack_t rack_t;
struct rack_t {
    char        *response;
    char        *cseq;
    char        *method;
};

typedef struct rseq_t rseq_t;
struct rseq_t {
    char        *response;
};
#endif

#ifdef SIP_EXT_CC_TRANSFER

typedef struct event_t sip_event_t;
struct event_t {
    char        *method;
};

typedef struct accept_encoding_t accept_encoding_t;
struct accept_encoding_t {
    char        *element;
    list_t      *gen_params;
};
typedef accept_encoding_t subscriptionstate_t;
#endif

#ifdef SIP_EXT_REPLACES
typedef accept_encoding_t replaces_t;
#endif


#ifdef SIP_EXT_SESSION_TIMER
typedef struct sessionexpires_t sessionexpires_t;
struct sessionexpires_t {
    char        *deltaseconds;
    list_t      *gen_params;
};
typedef sessionexpires_t minse_t;
#endif


typedef struct sip_t sip_t;
struct sip_t {
    startline_t *strtline;
    list_t      *accepts;
    list_t      *accept_encodings;
    list_t      *accept_languages;
    list_t      *alert_infos;
    list_t      *allows;
    list_t      *authorizations;
    call_id_t   *call_id;
    list_t      *call_infos;
    list_t      *contacts;
    list_t      *content_dispositions;
    list_t      *content_encodings;
    content_length_t *contentlength;
    content_type_t   *content_type;
    cseq_t      *cseq;
    list_t      *error_infos;
    from_t      *from;
#ifdef SIP_EXT_REMOTE_PRIVACY
    remotepartyid_t  *remotepartyid;
#endif
    mime_version_t   *mime_version;
#ifdef SIP_EXT_PRIVACY_ID
    list_t      *p_asserted_identities;
    list_t      *p_preferred_identities;
#endif
    list_t      *proxy_authenticates;
    list_t      *proxy_authorizations;
#ifdef SIP_EXT_100REL
    rack_t      *rack;
    rseq_t      *rseq;
#endif
    list_t      *record_routes;
    list_t      *routes;
    list_t      *requires;
    list_t      *supporteds;
    to_t        *to;
    list_t      *vias;
#ifdef SIP_EXT_CC_TRANSFER
    url_t       *refer_to;  
    sip_event_t *event;
    subscriptionstate_t *subscription_state;
#endif
#ifdef SIP_EXT_REPLACES
    replaces_t  *replaces;
#endif
#ifdef SIP_EXT_SESSION_TIMER
    sessionexpires_t *sessionexpires;
    minse_t     *minse;
#endif
    list_t      *www_authenticates;
    list_t      *headers;
    list_t      *bodies;
    /*
    * 1: structure and buffer "message" are identical.
    * 2: buffer "message" is not up to date with the
    *     structure info (call msg_2char to update it).
    */
    int         message_property;
    char        *message;
};

typedef struct sipevent_t sipevent_t;
struct sipevent_t {
    type_t      type;	/* 枚举类型 例如  RCV_REQINVITE  SND_REQINVITE */
    int         transactionid;
    sip_t       *sip;
};

//-----------------------------------------

extern sipevent_t *osip_parse(char *buf);

#define MSG_IS_RESPONSE(msg)    ((msg)->strtline->statuscode != NULL)
#define MSG_IS_REQUEST(msg)     ((msg)->strtline->statuscode == NULL)
#define MSG_IS_INVITE(msg)      (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "INVITE", 6))
#define MSG_IS_ACK(msg)         (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "ACK", 3))
#define MSG_IS_REGISTER(msg)    (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "REGISTER", 8))
#define MSG_IS_BYE(msg)         (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "BYE", 3))
#define MSG_IS_OPTIONS(msg)     (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "OPTIONS", 7))
#define MSG_IS_INFO(msg)        (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "INFO", 4))
#define MSG_IS_CANCEL(msg)      (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "CANCEL", 6))
#define MSG_IS_REFER(msg)       (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "REFER", 5))
#define MSG_IS_NOTIFY(msg)      (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "NOTIFY", 6))
#define MSG_IS_SUBSCRIBE(msg)   (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "SUBSCRIBE", 9))
#define MSG_IS_MESSAGE(msg)     (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "MESSAGE", 7))
#define MSG_IS_PRACK(msg)       (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "PRACK", 5))
#define MSG_IS_UPDATE(msg)      (MSG_IS_REQUEST(msg) && 0 == strncmp((msg)->strtline->sipmethod, "UPDATE", 6))
#define MSG_IS_STATUS_1XX(msg)  (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "1", 1))
#define MSG_IS_STATUS_2XX(msg)  (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "2", 1))
#define MSG_IS_STATUS_3XX(msg)  (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "3", 1))
#define MSG_IS_STATUS_4XX(msg)  (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "4", 1))
#define MSG_IS_STATUS_5XX(msg)  (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "5", 1))
#define MSG_IS_STATUS_6XX(msg)  (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "6", 1))
#if 1
#define MSG_IS_BUSY_HERE(msg)   (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "486", 3)) //486 Busy Here
#define MSG_IS_FORBIDDEN(msg)   (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "403", 3)) //403 Forbidden
#define MSG_IS_NOT_FOUND(msg)   (MSG_IS_RESPONSE(msg) && 0 == strncmp((msg)->strtline->statuscode, "404", 3)) //404 Not Found
#endif
#define MSG_TEST_CODE(msg, code)(MSG_IS_RESPONSE(msg) && code == (int)satoi((msg)->strtline->statuscode))
#define MSG_IS_RESPONSEFOR(msg, requestname)  (MSG_IS_RESPONSE(msg) && \
                                 0 == strcmp((msg)->cseq->method, requestname))



#endif
