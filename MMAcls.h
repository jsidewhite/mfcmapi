#pragma once
// MMPropTag.h : Prop tag parsing for MrMAPI

void DoAcls(_In_ MYOPTIONS ProgOpts);
void DumpExchangeTable(_In_z_ LPWSTR lpszProfile, _In_ ULONG ulPropTag, _In_ ULONG ulFolder);

enum {DEFAULT_UNSPECIFIED,
DEFAULT_CALENDAR,
DEFAULT_CONTACTS,
DEFAULT_JOURNAL,
DEFAULT_NOTES,
DEFAULT_TASKS,
DEFAULT_REMINDERS,
DEFAULT_DRAFTS,
DEFAULT_SENTITEMS,
DEFAULT_OUTBOX,
DEFAULT_DELETEDITEMS,
DEFAULT_FINDER,
DEFAULT_IPM_SUBTREE,
DEFAULT_INBOX,
NUM_DEFAULT_PROPS};

// Keep this in sync with the above enum
static LPSTR FolderNames[] = {
	"",
	"Calendar",
	"Contacts",
	"Journal",
	"Notes",
	"Tasks",
	"Reminders",
	"Drafts",
	"Sent Items",
	"Outbox",
	"Deleted Items",
	"Finder",
	"IPM_SUBTREE",
	"Inbox",
};