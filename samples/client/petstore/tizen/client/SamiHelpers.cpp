#include "SamiHelpers.h"
#include "SamiModelFactory.h"

using namespace Tizen::Base;
using namespace Tizen::Base::Utility;
using namespace Tizen::Base::Collection;
using namespace Tizen::Web::Json;
using namespace Tizen::Locales;
using namespace Tizen::System;

namespace Swagger {
  JsonObject*
  toJson(void* ptr, String type, String containerType) {
    if(ptr == null)
      return null;
    if(containerType != null && !type.Equals(L"", false)) {
      if(containerType.Equals(L"array", false)) {
        if(type != null) {
          IList* list = static_cast< IList* >(ptr);
          JsonArray* array = new JsonArray();
          int sz = list->GetCount();
          for(int i = 0; i < sz; i++) {
            void* itemAt = list->GetAt(i);
            JsonObject* item = toJson(itemAt, type, null);
            if(item != null)
              array->Add(item);
          }
          return (JsonObject*)array;
        }
        return null;
      }
    }
    else if(type.Equals(L"Boolean", false)) {
      Boolean* v = static_cast< Boolean* >(ptr);
      return (JsonObject*) new JsonBool(*v);
    }
    else if(type.Equals(L"String", false)) {
      String* v = static_cast< String* >(ptr);
      return (JsonObject*) new JsonString(*v);
    }
    else if(type.Equals(L"Integer", false)) {
      Integer* v = static_cast< Integer* >(ptr);
      return (JsonObject*) new JsonNumber(v->ToInt());
    }
    else if(type.Equals(L"Long", false)) {
      Long* v = static_cast< Long* >(ptr);
      return (JsonObject*) new JsonNumber(v->ToInt());
    }
    else if(type.Equals(L"Float", false)) {
      Float* v = static_cast< Float* >(ptr);
      return (JsonObject*) new JsonNumber(v->ToFloat());
    }
    else if(type.Equals(L"Double", false)) {
      Double* v = static_cast< Double* >(ptr);
      return (JsonObject*) new JsonNumber(v->ToDouble());
    }
    else if(type.Equals(L"DateTime", false)) {
      DateTime* v = static_cast< DateTime* >(ptr);
      DateTimeFormatter* pFormatter = DateTimeFormatter::CreateDateTimeFormatterN();
      String date;
      pFormatter->ApplyPattern(L"yyyy-MM-dd");
      pFormatter->Format(*v, date);

      String time;
      pFormatter->ApplyPattern(L"hh:mm:ss");
      pFormatter->Format(*v, time);
      String formattedString = date + "T" + time;
      delete pFormatter;
      return (JsonObject*)new JsonString(formattedString);
    }
    else if(type.StartsWith(L"Sami", 0)) {
      SamiObject* obj = static_cast< SamiObject* >(ptr);
      return obj->asJsonObject();
    }
    return null;
  }

  void
  toISO8601(String str, DateTime* dt) {
    int idx, start;
    int year, month, day, hour, minute, second;

    start = 0;
    str.IndexOf(L"-", start, idx);
    String yearString;
    str.SubString(0, idx, yearString);
    Integer::Parse(yearString, year);

    start = idx+1;
    str.IndexOf(L"-", start, idx);
    String monthString;
    str.SubString(start, idx - start, monthString);
    Integer::Parse(monthString, month);

    start = idx+1;
    str.IndexOf(L"T", start, idx);
    String dayString;
    str.SubString(start, idx - start, dayString);
    Integer::Parse(dayString, day);

    start = idx+1;
    str.IndexOf(L":", start, idx);
    if(idx > 0) {
      String hourString;
      str.SubString(start, idx - start, hourString);
      Integer::Parse(hourString, hour);
    }

    start = idx+1;
    str.IndexOf(L":", start, idx);
    if(idx > 0) {
      String minuteString;
      str.SubString(start, idx - start, minuteString);
      Integer::Parse(minuteString, minute);
    }

    start = idx+1;
    str.IndexOf(L"+", start, idx);
    if(idx > 0) {
      String secondString;
      str.SubString(start, idx - start, secondString);
      Integer::Parse(secondString, second);
    }

    dt->SetValue(year, month, day, hour, minute, second);
  }

  void
  jsonToValue(void* target, IJsonValue* ptr, String type, String innerType) {
    if(target == null || ptr == null) {
      return;
    }
    if(type.StartsWith(L"Boolean", 0)) {
      JsonBool* json = static_cast< JsonBool* >(ptr);
      Boolean* val = static_cast< Boolean* > (target);
      val->value = json->ToBool();
    }
    else if(type.StartsWith(L"String", 0)) {
      JsonString* json = static_cast< JsonString* >(ptr);
      String* val = static_cast< String* > (target);
      val->Clear();
      val->Append(json->GetPointer());
    }
    else if(type.StartsWith(L"Integer", 0)) {
      JsonNumber* json = static_cast< JsonNumber* >(ptr);
      Integer* val = static_cast< Integer* > (target);
      *val = json->ToInt();
    }
    else if(type.StartsWith(L"Long", 0)) {
      JsonNumber* json = static_cast< JsonNumber* >(ptr);
      Long* val = static_cast< Long* > (target);
      *val = json->ToLong();
    }
    else if(type.StartsWith(L"DateTime", 0)) {
      JsonString* json = static_cast< JsonString* >(ptr);
      String str;
      str.Append(json->GetPointer());

      DateTime* val = static_cast< DateTime* > (target);
      toISO8601(str, val);
    }
    else if(type.StartsWith(L"Sami", 0)) {
      SamiObject* obj = static_cast< SamiObject* > (target);
      obj->fromJsonObject(ptr);
    }
    else if(type.StartsWith(L"IList", 0)) {
      IList* obj = static_cast< IList* >(target);
      JsonArray* pJsonArray = static_cast< JsonArray* >(ptr);

      IEnumeratorT< IJsonValue* >* pEnum = pJsonArray->GetEnumeratorN();
      while (pEnum->MoveNext() == E_SUCCESS) {
        IJsonValue* pJsonValue = null;
        pEnum->GetCurrent(pJsonValue);
        void* updatedTarget = null;
        updatedTarget = create(innerType);

        if(updatedTarget != null) {
          jsonToValue(updatedTarget, pJsonValue, innerType, L"");
          obj->Add((Object*)updatedTarget);
        }
      }
      delete pEnum;
    }
  }

  Integer*
  jsonToInteger(IJsonValue* value) {
    if(value == null)
        return null;
    switch(value->GetType()) {
    case JSON_TYPE_STRING:
      break;
    case JSON_TYPE_NUMBER: {
      JsonNumber* number = static_cast< JsonNumber* >(value);
      int num = number->ToInt();
      return new Integer(num);
    }
    case JSON_TYPE_OBJECT:
      break;
    case JSON_TYPE_ARRAY:
      break;
    case JSON_TYPE_BOOL:
      break;
    case JSON_TYPE_NULL:
      break;
    }
    return null;
  }

  Long*
  jsonToLong(IJsonValue* value) {
    if(value == null)
        return null;
    switch(value->GetType()) {
    case JSON_TYPE_STRING:
      break;
    case JSON_TYPE_NUMBER: {
      JsonNumber* number = static_cast< JsonNumber* >(value);
      long int num = number->ToLong();
      return new Long(num);
    }
    case JSON_TYPE_OBJECT:
      break;
    case JSON_TYPE_ARRAY:
      break;
    case JSON_TYPE_BOOL:
      break;
    case JSON_TYPE_NULL:
      break;
    }
    return null;
  }

  String*
  jsonToString(IJsonValue* value) {
    if(value == null)
        return null;
    switch(value->GetType()) {
    case JSON_TYPE_STRING: {
      JsonString* string = static_cast< JsonString* >(value);
      return new String(string->GetPointer());
    }
    case JSON_TYPE_NUMBER:
      break;
    case JSON_TYPE_OBJECT:
      break;
    case JSON_TYPE_ARRAY:
      break;
    case JSON_TYPE_BOOL:
      break;
    case JSON_TYPE_NULL:
      break;
    }
    return null;
  }

  String
  stringify(void* ptr, String type) {
    if(type.StartsWith(L"String", 0)) {
      String * str = static_cast< String* > (ptr);
      return String(str->GetPointer());
    }
    if(type.StartsWith(L"Integer", 0)) {
      Integer* pInt = static_cast< Integer* > (ptr);
      return pInt->ToString();
    }
    if(type.StartsWith(L"Long", 0)) {
      Long* pLong = static_cast< Long* > (ptr);
      return pLong->ToString();
    }
    return L"";
  }
} /* namespace Swagger */
