#from django.http import HttpResponse, JsonResponse
#from django.views.decorators.csrf import csrf_exempt
#from rest_framework.parsers import JSONParser
#from moistureduino.models import Entry
#from moistureduino.serializers import EntrySerializer

#from rest_framework import renderers
#from rest_framework.decorators import api_view
#from rest_framework.response import Response
#from rest_framework.reverse import reverse
#from rest_framework.generics import GenericAPIView

#@api_view(['GET'])
#@csrf_exempt
#def api_root(request):
    #queryset = Entry.objects.all()
    #return Response({
        #'entries': reverse('entry-list', request=request)
    #})



#class EntryDisplay(GenericAPIView):
    #queryset = Entry.objects.all()
    #renderer_classes = [renderers.StaticHTMLRenderer]

    #@csrf_exempt
    #def get(self, request, *args, **kwargs):
        #entry = self.get_object()
        #return Response(entry)

#@csrf_exempt
#def entry_list(request):
    #"""
    #List all moisture sensor entries, or create a new entry.
    #"""
    #queryset = Entry.objects.all()
    #if request.method == 'GET':
        #entries = Entry.objects.all()
        #serializer = EntrySerializer(entries, many=True)
        #return JsonResponse(serializer.data, safe=False)

    #elif request.method == 'POST':
        #data = JSONParser().parse(request)
        #serializer = EntrySerializer(data=data)
        #if serializer.is_valid():
            #serializer.save()
            #return JsonResponse(serializer.data, status=201)
        #return JsonResponse(serializer.errors, status=400)

#@csrf_exempt
#def entry_detail(request, pk):
    #"""
    #Retrieve, update or delete a moisture sensor entry.
    #"""
    #try:
        #entry = Entry.objects.get(pk=pk)
    #except Entry.DoesNotExist:
        #return HttpResponse(status=404)

    #if request.method == 'GET':
        #serializer = EntrySerializer(entry)
        #return JsonResponse(serializer.data)

    #elif request.method == 'PUT':
        #data = JSONParser().parse(request)
        #serializer = EntrySerializer(entry, data=data)
        #if serializer.is_valid():
            #serializer.save()
            #return JsonResponse(serializer.data)
        #return JsonResponse(serializer.errors, status=400)

    #elif request.method == 'DELETE':
        #entry.delete()
        #return HttpResponse(status=204)


from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response
from moistureduino.models import Entry
from moistureduino.serializers import EntrySerializer


@api_view(['GET', 'POST'])
def entry_list(request, format=None):
    """
    List all code entries, or create a new entry.
    """
    if request.method == 'GET':
        entries = Entry.objects.all()
        serializer = EntrySerializer(entries, many=True)
        return Response(serializer.data)

    elif request.method == 'POST':
        serializer = EntrySerializer(data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


@api_view(['GET', 'PUT', 'DELETE'])
def entry_detail(request, pk, format=None):
    """
    Retrieve, update or delete a code entry.
    """
    try:
        entry = Entry.objects.get(pk=pk)
    except Entry.DoesNotExist:
        return Response(status=status.HTTP_404_NOT_FOUND)

    if request.method == 'GET':
        serializer = EntrySerializer(entry)
        return Response(serializer.data)

    elif request.method == 'PUT':
        serializer = EntrySerializer(entry, data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

    elif request.method == 'DELETE':
        entry.delete()
        return Response(status=status.HTTP_204_NO_CONTENT)
