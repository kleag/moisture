from django.http import HttpResponse, JsonResponse
from django.views.decorators.csrf import csrf_exempt
from rest_framework.parsers import JSONParser
from moistureduino.models import Entry
from moistureduino.serializers import EntrySerializer

@csrf_exempt
def entry_list(request):
    """
    List all moisture sensor entries, or create a new entry.
    """
    if request.method == 'GET':
        entries = Entry.objects.all()
        serializer = EntrySerializer(entries, many=True)
        return JsonResponse(serializer.data, safe=False)

    elif request.method == 'POST':
        data = JSONParser().parse(request)
        serializer = EntrySerializer(data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data, status=201)
        return JsonResponse(serializer.errors, status=400)

@csrf_exempt
def entry_detail(request, pk):
    """
    Retrieve, update or delete a moisture sensor entry.
    """
    try:
        entry = Entry.objects.get(pk=pk)
    except Entry.DoesNotExist:
        return HttpResponse(status=404)

    if request.method == 'GET':
        serializer = EntrySerializer(entry)
        return JsonResponse(serializer.data)

    elif request.method == 'PUT':
        data = JSONParser().parse(request)
        serializer = EntrySerializer(entry, data=data)
        if serializer.is_valid():
            serializer.save()
            return JsonResponse(serializer.data)
        return JsonResponse(serializer.errors, status=400)

    elif request.method == 'DELETE':
        entry.delete()
        return HttpResponse(status=204)

